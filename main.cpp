#include <igl/opengl/glfw/Viewer.h>
#include <igl/readOBJ.h>
#include <iostream>
// #include <stylization.hpp>
// #include "stylization.hpp"
#include <stylization.hpp>

using namespace std;
  

int main(int argc, char *argv[])
{

  Eigen::MatrixXd V,TC,CN;
  Eigen::MatrixXi F,FTC,FN;
  string arg;
  if ( argc == 0 || argv[1]==NULL || strlen(argv[1]) == 0 ){
    cout << "no file path specified, defaulting to bunny" << "\n";
    arg = "meshes/bunny.obj";
  }
  else{
  arg= argv[1];
  } 
  

  igl::readOBJ(arg, V, TC,CN,F,FTC,FN);
  Stylization style(V,F);



  for(int i=0;i<5;i++){
    cout<<i<<'\n';
    style.local_update();
    style.global_update();
  }

    igl::opengl::glfw::Viewer viewer;
  viewer.data().set_mesh(style.V_desired, F);
  viewer.data().set_face_based(true);
  viewer.launch();
  
}
