# 3D Face Alignment

**Introduction**

DirectX based tool for viewing output files from Microsofts Deep-3D-Face-Reconstruction project that takes a 2D image of a face and outputs a 3D mesh, cropped image and landmarks files (eyes, nose and mouth). The tool displays the cropped image in the top-left quadrant. The 3D mesh, with the cropped image projected on to it, is displayed in the top-right quadrant. A shadow mapping technique is used to highlight in red the part of the mesh that is occluded in the original image (bottom-left quadrant).The 2D landmark coordinates for the left and right eye are projected from texture space to world space and displayed in blue. These landmarks are used to rotate the image (Yaw and Roll) and align it with the camera as can be seen in the bottom right quadrant.

To compile the software use VS 2019 with MFC enabled, and the June 2010 DirectX SDK.
To use the software, open a .mesh file from the sample data folder.

The tool was written to explore the possibility of using computer vision techniques to assist in testing vision impairment in young children. Certain eyes tests are made difficult with children who are too young to keep their face calibrated with the testing equipment. By dynamically creating a 3D model of their face and rotating that towards the camera it may be possible to reduce the extent of this problem. 




![alt text](https://github.com/nodecomplete/FaceView/blob/master/FaceView/ScreenShot.jpg)



 
