#include <igl/opengl/glfw/Viewer.h>
#include <igl/readOBJ.h>


int main(int argc, char *argv[])
{

  Eigen::MatrixXd V,TC,CN;
  Eigen::MatrixXi F,FTC,FN;


  igl::readOBJ("meshes/bunny.obj", V, TC,CN,F,FTC,FN);

  
  // Plot the mesh
  igl::opengl::glfw::Viewer viewer;
  viewer.data().set_mesh(V, F);
  viewer.data().set_face_based(true);
  viewer.launch();
}
