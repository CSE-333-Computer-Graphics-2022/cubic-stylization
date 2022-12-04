#include <igl/opengl/glfw/Viewer.h>
#include <igl/readOBJ.h>
#include <iostream>
using namespace std;
  

int main(int argc, char *argv[])
{

  Eigen::MatrixXd V,TC,CN;
  Eigen::MatrixXi F,FTC,FN;
  string arg;
  if ( argc == 0 || argv[1]==NULL || strlen(argv[1]) == 0 ){
    cout << "no file path specified, defaulting to bunny" << "\n";
    arg = "meshes/bunny.obj";
  }else{
  arg= argv[1];
  } 
  
  
  igl::readOBJ(arg, V, TC,CN,F,FTC,FN);
  
  
  // Plot the mesh
  igl::opengl::glfw::Viewer viewer;
  viewer.data().set_mesh(V, F);
  viewer.data().set_face_based(true);
  viewer.launch();
}
