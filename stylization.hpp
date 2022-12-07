#include <eigen3/Eigen/Core>
#include <igl/min_quad_with_fixed.h>

class Stylization{
    private:
    Eigen::MatrixXd V;
    Eigen::MatrixXi F;
    std::vector<std::set<int>> neighbours;
    Eigen::MatrixXd R_all;
    igl::min_quad_with_fixed_data<double> solver_data;

    Eigen::SparseMatrix<double> L;
    public:
    Stylization(Eigen::MatrixXd V);
    ~Stylization();

};