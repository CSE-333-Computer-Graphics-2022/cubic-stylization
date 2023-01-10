# Cubic Stylization

This repository contains an implementation of the paper "Cubic Stylization" by Hsueh-Ti Derek Liu and Alec Jacobson, published in the ACM Transactions on Graphics in November 2019. The paper presents a 3D stylization algorithm that can turn an input shape into the style of a cube while maintaining the content of the original shape.

The key insight of the paper is that cubic style sculptures can be captured by the as-rigid-as-possible energy with an ℓ1-regularization on rotated surface normals. By minimizing this energy, the algorithm is able to produce a detail-preserving, cubic geometry. The method does not require any mesh surgery and can be used as a non-realistic modeling tool with artistic controls.

In this repository, we provide a modified version of the algorithm, implemented using ARAP energy. We have also included example usage in the form of scripts that showcase the algorithm on various 3D assets obtained from sketchfab.com, images of Cubic sculptures like “The Kiss” by Constantin Brâncusi, and “Taichi” by Ju Ming.

# Usage

1. Clone the repository to your local machine
2. make a build folder.
3. change directory to the build directory and run ` cmake ..`
4. Run ` make` in the build directory.
5. Execute the binary, followed by a path to an obj file that you want to cubify.
# Results


The algorithm is able to turn an input shape into the style of a cube while maintaining the content of the original shape. Texture and geometric features of the input shape are preserved in the stylized output.

![Uncubified](https://user-images.githubusercontent.com/29100891/211583410-235e945a-db02-41c2-9544-98c2668ae2f5.png =250x250)
![Cubified](https://user-images.githubusercontent.com/29100891/211583423-be8958fd-17fe-4c80-85a1-80bfcccd126b.png =250x250)

# Licensing!


This code is released under the MPL 2.0 License.
