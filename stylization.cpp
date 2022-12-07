#include <eigen3/Eigen/Core>
#include <igl/all.h>
#include <igl/cotmatrix.h>
#include <igl/min_quad_with_fixed.h>
#include <igl/readOBJ.h>
#include <set>
#include <vector>

using namespace std;

class Stylization {
private:
  Eigen::SparseMatrix<double> L;
  std::vector<std::set<int>> neighborsAll;
  igl::min_quad_with_fixed_data<double> solver_data;
  void precompute(Eigen::MatrixXd V, Eigen::MatrixXi F) {

    // precompute for quad solve
    igl::cotmatrix(V, F, L);
    Eigen::VectorXi b;

    igl::min_quad_with_fixed_precompute(L, b, Eigen::SparseMatrix<double>(),
                                        false, solver_data);


//    or (int i = 0; i < M.rows(); i++)
//  {
//    // Create a 3D vector from the ith row of the matrix
//    Eigen::Vector3d vec = M.row(i);
//
//    // Print the vector
//    std::cout << "Vector " << i << ": " << vec.transpose() << std::endl;
//  }
    for(int i = 0;i<V.rows();i++){
        std::set<int> empty_set;
        neighborsAll.push_back(empty_set);
    }
        for(int f = 0;f<F.rows();f++){
        neighborsAll[F.coeff(f,0)].insert(F.coeff(f,1));
        neighborsAll[F.coeff(f,0)].insert(F.coeff(f,2));

        neighborsAll[F.coeff(f,1)].insert(F.coeff(f,0));
        neighborsAll[F.coeff(f,1)].insert(F.coeff(f,2));
        
        neighborsAll[F.coeff(f,2)].insert(F.coeff(f,0));
        neighborsAll[F.coeff(f,2)].insert(F.coeff(f,1));

    }
  };

public:
  void local_update();
  void global_update(std::vector<Eigen::MatrixXd> RotationMatrices,
                     Eigen::MatrixXd Vertices,
                     Eigen::MatrixXd UpdatedVertices) {

    // form  linear system from Sorkine

    // init RHS storage matrix
    Eigen::MatrixXd RHSMatrix(Vertices.rows(), 3);

    // compute RHS for each vertex;
    for (int i = 0; i < Vertices.rows(); i++) {

      Eigen::Vector3d rhs_vertex_sum(0, 0, 0);

      for (auto j : neighborsAll[i]) {
        rhs_vertex_sum +=
            L.coeff(i, j) * ((RotationMatrices[i] + RotationMatrices[j]) *
                             (Vertices.row(i) - Vertices.row(j)));
      }

      RHSMatrix.row(i) = rhs_vertex_sum;
    }
    Eigen::VectorXd bc;
    // second part of quadratic optimization using igl
    for (int col = 0; col < Vertices.cols(); col++) {
      Eigen::VectorXd Uc, Bc, bcc;

      Bc = RHSMatrix.block(col * Vertices.rows(), 0, Vertices.rows(), 1);
      bcc =bc.col(col);
      min_quad_with_fixed_solve(solver_data, Bc, bcc, Eigen::VectorXd(), Uc);
      UpdatedVertices.col(col) = Uc;
    }
  }
};
