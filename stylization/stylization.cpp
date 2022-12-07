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
    }

    lambda = 1.0;
    Eigen::VectorXi b;
    igl::min_quad_with_fixed_precompute(L, b, Eigen::SparseMatrix<double>(), false, solver_data);

}

void Stylization::local_update(){
    for(int i = 0;i<V.rows();i++){
        Eigen::MatrixXd ni = normals.row(i).transpose();
        Eigen::MatrixXd zi = z.col(i);
        Eigen::MatrixXd ui = u.col(i);
        double rhoi = rho.coeff(1,i);
        Eigen::Matrix3d R;
        R << R_all.col(i*3), R_all.col(i*3+1), R_all.col(i*3+2);

        

        Eigen::MatrixXd Di(3,triangle_adjacency[i].size()*3);
        Eigen::MatrixXd Di_tilde(3,triangle_adjacency[i].size()*3);

        for(int ii=0;ii<triangle_adjacency[i].size();ii++){
            int face_index = triangle_adjacency[i][ii];
            Di.col(ii*3) = (V.row(F.coeff(face_index,0)) - V.row(F.coeff(face_index,1))).transpose();
            Di.col(ii*3+1) = (V.row(F.coeff(face_index,1)) - V.row(F.coeff(face_index,2))).transpose();
            Di.col(ii*3+2) = (V.row(F.coeff(face_index,2)) - V.row(F.coeff(face_index,0))).transpose();


            Di_tilde.col(ii*3) = (V_desired.row(F.coeff(face_index,0)) - V_desired.row(F.coeff(face_index,1))).transpose();
            Di_tilde.col(ii*3+1) = (V_desired.row(F.coeff(face_index,1)) - V_desired.row(F.coeff(face_index,2))).transpose();
            Di_tilde.col(ii*3+2) = (V_desired.row(F.coeff(face_index,2)) - V_desired.row(F.coeff(face_index,0))).transpose();
        }

        Eigen::MatrixXd Wi(triangle_adjacency[i].size()*3,triangle_adjacency[i].size()*3);
        Wi.setZero();

        for(int ii=0;ii<triangle_adjacency[i].size();ii++){
            int face_index = triangle_adjacency[i][ii];
            Wi.coeffRef(3*ii,3*ii) = L.coeff(F.coeff(face_index,0), F.coeff(face_index,1));
            Wi.coeffRef(3*ii+1,3*ii+1) = L.coeff(F.coeff(face_index,1), F.coeff(face_index,2));
            Wi.coeffRef(3*ii+2,3*ii+2) = L.coeff(F.coeff(face_index,2), F.coeff(face_index,0));
        }


        Eigen::MatrixXd Mconst = Di * Wi * Di_tilde.transpose();

        for(int ii=0;ii<50;i++){
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

            double k = lambda*V_area.coeff(1,i)/rhoi;
            Eigen::MatrixXd x = R*ni + ui;

            Eigen::VectorXd tmp1 = x.array() - k;
            Eigen::VectorXd posMax = tmp1.array().max(0.0);

            Eigen::VectorXd tmp2 = -x.array() - k;
            Eigen::VectorXd negMax = tmp2.array().max(0.0);

            Eigen::MatrixXd zOld(zi);
            zi = posMax - negMax;


            ui.noalias() += R*ni - zi;



            double r_norm = (zi - R*ni).norm();
            double s_norm = (-rho * (zi - zOld)).norm();
            
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
        z.col(i) = zi;
        u.col(i) = ui;
        rho.coeffRef(1,i) = rhoi;

        R_all.block(0,i*3,3,3) = R;
        for(int i=0;i<V.rows();i++){
            Eigen::Matrix3d R_temp;
            R_temp<<R_all.col(i*3), R_all.col(i*3+1), R_all.col(i*3+2);
            R_all_vect[i] = R_temp;
        }
    }
}



void Stylization::global_update() {

    // form  linear system from Sorkine

    // init RHS storage matrix
    Eigen::MatrixXd RHSMatrix(V.rows(), 3);

    // compute RHS for each vertex;
    for (int i = 0; i < V.rows(); i++) {

      Eigen::Vector3d rhs_vertex_sum(0, 0, 0);

      for (auto j : adjList[i]) {
        rhs_vertex_sum +=
            L.coeff(i, j) * ((R_all_vect[i] + R_all_vect[j]) *
                             (V.row(i) - V.row(j)));
      }

      RHSMatrix.row(i) = rhs_vertex_sum;
    }
    Eigen::VectorXd bc;
    // second part of quadratic optimization using igl
    for (int col = 0; col < V.cols(); col++) {
      Eigen::VectorXd Uc, Bc, bcc;

      Bc = RHSMatrix.block(col * V.rows(), 0, V.rows(), 1);
      bcc =bc.col(col);
      min_quad_with_fixed_solve(solver_data, Bc, bcc, Eigen::VectorXd(), Uc);
      V_desired.col(col) = Uc;
    }
  }