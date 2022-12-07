#include <eigen3/Eigen/Core>

class Stylization{
    private:
    Eigen::MatrixXd V;
    Eigen::MatrixXi F;
    Eigen::MatrixXd R_all;
    Eigen::MatrixXd L;
    public:
    Stylization(Eigen::MatrixXd V);
    ~Stylization();

};