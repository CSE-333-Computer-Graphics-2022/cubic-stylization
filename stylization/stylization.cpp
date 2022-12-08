#include "stylization.hpp"
#include <iostream>

Stylization::Stylization(Eigen::MatrixXd& _V,Eigen::MatrixXi& _F) : V(_V), F(_F), V_desired(_V)
{
    igl::per_vertex_normals(V,F,normals);
    igl::cotmatrix(V,F,L);
    Eigen::SparseMatrix<double> area;
    igl::massmatrix(V,F,igl::MASSMATRIX_TYPE_BARYCENTRIC,area);
    V_area = area.diagonal();

    igl::adjacency_list(F,adjList);
    vector< vector< int > > VI;
    igl::vertex_triangle_adjacency(V.rows(),F,triangle_adjacency,VI);
    
    z.resize(3,V.rows());z.setRandom();
    u.resize(3,V.rows());u.setRandom();
    rho.resize(3,V.rows());rho.setConstant(1e-3);
    R_all.resize(3,3*V.rows());
    for(int i=0;i<V.rows();i++){
        R_all_vect.push_back(Eigen::MatrixXd::Identity(3,3));
        R_all.col(i*3) = R_all_vect[i].col(0);
        R_all.col(i*3+1) = R_all_vect[i].col(1);
        R_all.col(i*3+2) = R_all_vect[i].col(2);
    }

    lambda = 2.0;
    Eigen::VectorXi b;
    igl::min_quad_with_fixed_precompute(L, b, Eigen::SparseMatrix<double>(), false, solver_data);


    for(int i = 0;i<V.rows();i++){
        D.push_back(Eigen::MatrixXd(3,triangle_adjacency[i].size()*3));
        D_tilde.push_back(Eigen::MatrixXd(3,triangle_adjacency[i].size()*3));
        W.push_back(Eigen::MatrixXd(triangle_adjacency[i].size()*3,triangle_adjacency[i].size()*3));
        W[i].setZero();

        for(int ii=0;ii<triangle_adjacency[i].size();ii++){
            int face_index = triangle_adjacency[i][ii];
            D[i].col(ii*3) = (V.row(F.coeff(face_index,0)) - V.row(F.coeff(face_index,1))).transpose();
            D[i].col(ii*3+1) = (V.row(F.coeff(face_index,1)) - V.row(F.coeff(face_index,2))).transpose();
            D[i].col(ii*3+2) = (V.row(F.coeff(face_index,2)) - V.row(F.coeff(face_index,0))).transpose();


            D_tilde[i].col(ii*3) = (V_desired.row(F.coeff(face_index,0)) - V_desired.row(F.coeff(face_index,1))).transpose();
            D_tilde[i].col(ii*3+1) = (V_desired.row(F.coeff(face_index,1)) - V_desired.row(F.coeff(face_index,2))).transpose();
            D_tilde[i].col(ii*3+2) = (V_desired.row(F.coeff(face_index,2)) - V_desired.row(F.coeff(face_index,0))).transpose();
        }

        for(int ii=0;ii<triangle_adjacency[i].size();ii++){
            int face_index = triangle_adjacency[i][ii];
            W[i].coeffRef(3*ii,3*ii) = L.coeff(F.coeff(face_index,0), F.coeff(face_index,1));
            W[i].coeffRef(3*ii+1,3*ii+1) = L.coeff(F.coeff(face_index,1), F.coeff(face_index,2));
            W[i].coeffRef(3*ii+2,3*ii+2) = L.coeff(F.coeff(face_index,2), F.coeff(face_index,0));
        }

    }
}


void Stylization::local_update(){

    igl::parallel_for(V.rows(),
    [this](const int i){
        Eigen::MatrixXd ni = normals.row(i).transpose();
        Eigen::MatrixXd zi = z.col(i);
        Eigen::MatrixXd ui = u.col(i);
        double rhoi = rho.coeff(1,i);
        Eigen::Matrix3d R;
        R << R_all.col(i*3), R_all.col(i*3+1), R_all.col(i*3+2);

        




        Eigen::MatrixXd Mconst = D[i] * W[i] * D_tilde[i].transpose();
        // std::cout<<"going in "<<i<<"\n";

        for(int ii=0;ii<1;ii++){
            Eigen::MatrixXd M = Mconst + rhoi*ni*(zi-ui).transpose();

            Eigen::JacobiSVD<Eigen::Matrix3d> svd;
            svd.compute(M, Eigen::ComputeFullU | Eigen::ComputeFullV);
            Eigen::Matrix3d U = svd.matrixU();
            Eigen::Matrix3d V = svd.matrixV();
            R = V * U.transpose();
            if (R.determinant() < 0)
            {
                U.col(2) = -U.col(2);
                R = V * U.transpose();
            }

            double k = lambda*V_area.coeff(i,1)/rhoi;
            Eigen::MatrixXd x = R*ni + ui;

            Eigen::VectorXd tmp1 = x.array() - k;
            Eigen::VectorXd posMax = tmp1.array().max(0.0);

            Eigen::VectorXd tmp2 = -x.array() - k;
            Eigen::VectorXd negMax = tmp2.array().max(0.0);

            Eigen::MatrixXd zOld(zi);
            zi = posMax - negMax;


            ui.noalias() += R*ni - zi;

            double r_norm = (zi - R*ni).norm();
            double s_norm = (-rhoi * (zi - zOld)).norm();


            // rho step
            if (r_norm > 10 * s_norm)
            {
                rhoi = 2 * rhoi;
                ui = ui / 2;
            }
            else if (s_norm > 10 * r_norm)
            {
                rhoi = rhoi / 2;
                ui = ui * 2;
            }

        }
        // std::cout<<"coming out\n";
        z.col(i) = zi;
        u.col(i) = ui;
        rho.coeffRef(1,i) = rhoi;

        R_all.block(0,i*3,3,3) = R;


    }
    ,1000);
            for(int i=0;i<V.rows();i++){
            Eigen::Matrix3d R_temp;
            R_temp<<R_all.col(i*3), R_all.col(i*3+1), R_all.col(i*3+2);
            R_all_vect[i] = R_temp;
        }
}



void Stylization::global_update() {

    // form  linear system from Sorkine

    // init RHS storage matrix
    Eigen::MatrixXd RHSMatrix(V.rows(), 3);

    // compute RHS for each vertex;
    for (int i = 0; i < V.rows(); i++) {

      Eigen::MatrixXd rhs_vertex_sum(3,1);
      rhs_vertex_sum << 0.0, 0.0, 0.0;
        // std::cout<<i<<"loop start\n";
      for (auto j : adjList[i]) {
                // std::cout << "coeff extract";

        double coeff =  L.coeff(i, j);
        // std::cout << "coeff extract done";

        Eigen::MatrixXd rows_diff = (V.row(i) - V.row(j)).transpose();  
        // std::cout << "t1";
        Eigen::MatrixXd rot_diff= (R_all_vect[i] + R_all_vect[j]) ;
        // std::cout << "t2";
        
        Eigen::MatrixXd product = rot_diff*rows_diff;
        // std::cout << "t3";
        
        product*=coeff;  
        rhs_vertex_sum +=  product ;
      }

      RHSMatrix.row(i) = rhs_vertex_sum.transpose();
    }
    Eigen::VectorXd bc;
    // second part of quadratic optimization using igl
    for (int col = 0; col < V.cols(); col++) {
      Eigen::VectorXd Uc, Bc, bcc;

      Bc = RHSMatrix.block(0, col, V.rows(), 1);
      //bcc =bc.col(col);
      min_quad_with_fixed_solve(solver_data, Bc, bcc, Eigen::VectorXd(), Uc);
      V_desired.col(col) = Uc;
    }
  }