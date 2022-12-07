#include "stylization.hpp"


Stylization::Stylization(Eigen::MatrixXd& _V,Eigen::MatrixXi& _F) : V(_V), F(_F)
{
    igl::per_vertex_normals(V,F,normals);
    igl::cotmatrix(V,F,L);
    Eigen::SparseMatrix<double> area;
    igl::massmatrix(V,F,igl::MASSMATRIX_TYPE_BARYCENTRIC,area);
    V_area = area.diagonal();

    igl::adjacency_list(F,adjList);
    z.resize(3,V.rows());z.setRandom();
    u.resize(3,V.rows());u.setRandom();
    rho.resize(3,V.rows());rho.setConstant(1e-3);
    V_desired.resizeLike(V);
    V_desired.setRandom();

}

void Stylization::local_update(){
    for(int i = 0;i<V.rows();i++){
        Eigen::MatrixXd ni = normals.row(i).transpose();
        Eigen::MatrixXd zi = z.col(i);
        Eigen::MatrixXd ui = u.col(i);
        double rhoi = rho.coeff(1,i);
        Eigen::Matrix3d R;
        R << R_all.col(i*3), R_all.col(i*3+1), R_all.col(i*3+2);

        Eigen::MatrixXd Di(3,adjList[i].size());
        Eigen::MatrixXd Di_tilde(3,adjList[i].size());
        for(int ii=0;ii<adjList[i].size();ii++){
            Di << V.row(adjList[i][ii]).transpose() - V.row(i).transpose();
            Di_tilde << V_desired.row(adjList[i][ii]).transpose() - V_desired.row(i).transpose();
        }
        Eigen::MatrixXd Wi(adjList[i].size(),adjList[i].size());
        for(int ii=0;ii<adjList[i].size();ii++){
            Wi << L.coeff(i,adjList[i][ii]);
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

            Eigen::MatrixXd zOld = z;
            z = posMax - negMax;


            ui.noalias() += R*ni - zi;

            double r_norm = (z - R*ni).norm();
            double s_norm = (-rho * (z - zOld)).norm();
            
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
    }
}