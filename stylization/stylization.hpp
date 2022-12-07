// #include <eigen3/Eigen/Core>
#include <vector>
#include <igl/opengl/glfw/Viewer.h>
#include <igl/readOBJ.h>
#include <igl/cotmatrix.h>
#include <igl/massmatrix.h>
#include <igl/per_vertex_normals.h>
#include <igl/arap_rhs.h>
#include <igl/columnize.h>
#include <igl/slice.h>
#include <igl/adjacency_list.h>

using std::vector;


class Stylization{
    private:
    Eigen::MatrixXd V;
    Eigen::MatrixXd V_desired;
    Eigen::MatrixXd V_area;
    Eigen::MatrixXi F;
    Eigen::MatrixXd R_all;
    Eigen::SparseMatrix<double> L;
    Eigen::MatrixXd normals;
    vector< vector< int > > adjList;

    //Parameters required for admm
    Eigen::MatrixXd z;
    Eigen::MatrixXd u;
    Eigen::MatrixXd rho;

    public:
    double lambda;
    Stylization(Eigen::MatrixXd& _V, Eigen::MatrixXi& _F);

    void local_update();
};