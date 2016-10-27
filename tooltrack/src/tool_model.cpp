#include <ros/ros.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <stdio.h>
#include <boost/random.hpp>
#include <boost/random/normal_distribution.hpp>
#include <iostream>
#include <cwru_opencv_common/projective_geometry.h>

#include <tool_tracking_lib/tool_model.h>
#include <math.h>

using cv_projective::reprojectPoint;
boost::mt19937 rng(time(0));
using namespace std;

//constructor
ToolModel::ToolModel(){

////////
	for(int i(0); i<3; i++){
		for(int j(0);j<3;j++){
			I(i)(j) = 0.0;
		}
	}
	I(1)(1) = 1;
	I(2)(2) = 1;
	I(3)(3) = 1;
///////
};

double ToolModel::randomNumber(double stdev, double mean){
	boost::normal_distribution<> nd(mean, stdev);
	boost::variate_generator<boost::mt19937&, boost::normal_distribution<> > var_nor(rng, nd);
	double d = var_nor();

	return d;

}

//set zero configuratio for tool points;
ToolModel::load_model_vertices( std::vector< glm::vec3 > &out_vertices ){

    printf("Loading OBJ file %s...\n", path);

    std::vector< unsigned int > vertexIndices, uvIndices, normalIndices;
    //std::vector< unsigned int > vertexIndices;
	std::vector< glm::vec3 > temp_vertices;
	std::vector< glm::vec2 > temp_uvs;
	std::vector< glm::vec3 > temp_normals;

    FILE * file = fopen(path, "r");
    if( file == NULL ){
        printf("Impossible to open the file ! Are you in the right path ? See Tutorial 1 for details\n");
        return false;
    }

    while( 1 ){

        char lineHeader[128];
        // read the first word of the line
        int res = fscanf(file, "%s", lineHeader);
        if (res == EOF)
            break; // EOF = End Of File. Quit the loop.

        // else : parse lineHeader
        
        if ( strcmp( lineHeader, "v" ) == 0 ){
            glm::vec3 vertex;
            
            fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z );
            // cout<<"vertex x"<< vertex.x<<endl;
            // cout<<"vertex y"<<vertex.y<<endl;
            // cout<<"vertex z"<<vertex.z<<endl;
            temp_vertices.push_back(vertex);
        }

        else if ( strcmp( lineHeader, "vt" ) == 0 ){
	    glm::vec2 uv;
	    fscanf(file, "%f %f\n", &uv.x, &uv.y );
	    // cout<<"uv"<<uv.x<<endl;
	    temp_uvs.push_back(uv);
		} 
        else if ( strcmp( lineHeader, "vn" ) == 0 ){
            glm::vec3 normal;
            fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z );
            // cout<<"normal x"<< normal.x<<endl;
            // cout<<"normal y"<< normal.y<<endl;
            // cout<<"normal z"<< normal.z<<endl;
            temp_normals.push_back(normal);
        }
        else if ( strcmp( lineHeader, "f" ) == 0 ){
    std::string vertex1, vertex2, vertex3;
    unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
    int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2] );
    if (matches != 9){
        printf("File can't be read by our simple parser : ( Try exporting with other options\n");
        return false;
    }
    vertexIndices.push_back(vertexIndex[0]);
    vertexIndices.push_back(vertexIndex[1]);
    vertexIndices.push_back(vertexIndex[2]);
    uvIndices    .push_back(uvIndex[0]);
    uvIndices    .push_back(uvIndex[1]);
    uvIndices    .push_back(uvIndex[2]);
    normalIndices.push_back(normalIndex[0]);
    normalIndices.push_back(normalIndex[1]);
    normalIndices.push_back(normalIndex[2]);
            
        }
    }

    // For each vertex of each triangle
    for( unsigned int i=0; i<vertexIndices.size(); i++ ){

        // Get the indices of its attributes
        unsigned int vertexIndex = vertexIndices[i];
        
        // Get the attributes thanks to the index
        glm::vec3 vertex = temp_vertices[ vertexIndex-1 ];
        
        // Put the attributes in buffers
        out_vertices.push_back(vertex);
    
    }

    cout<<"size"<< out_vertices.size()<<endl;
    printf("loaded file %s successfully.\n", path);

    //return true;
};

toolModel ToolModel::setRandomConfig(const toolModel &initial, double stdev, double mean)
{
	// copy initial toolModel
	toolModel newTool = initial;

	//create normally distributed random number with the given stdev and mean

	//TODO: pertub all translation components
	newTool.tvec(0) += randomNumber(stdev,mean); 
	newTool.tvec(1) += randomNumber(stdev,mean);
	newTool.tvec(2) += randomNumber(stdev,mean);
	
	double angle = randomNumber(stdev,mean);
	newTool.rvec(0) += (angle/10.0)*1000.0; //rotation on x axis +/-5 degrees

	angle = randomNumber(stdev,mean);
	newTool.rvec(0) += (angle/10.0)*1000.0; ////rotation on x axis +/-5 degrees

	angle = randomNumber(stdev,mean);
	newTool.rvec(2) += (angle/10.0)*1000.0; //rotation on z axis +/-5 degrees


    /**************smaple the angles of the joints**************/
	//-90,90//
	angle = randomNumber(stdev,mean);
	newTool.theta_ellipse += (angle/10.0)*1000.0;
	if (newTool.theta_ellipse < -M_PI/2 || newTool.theta_ellipse > M_PI/2)   //use M_PI HERE
		newTool.theta_ellipse = randomNumber(stdev,mean);

	// lets a assign the upside one 1, and set positive as clockwise 
	angle = randomNumber(stdev,mean);
	newTool.theta_grip_1 += (angle/10.0)*1000.0;
	if (newTool.theta_grip_1 < -1.2*M_PI/2 || newTool.theta_grip_1 > 1.2*M_PI/2)   //use M_PI HERE
		newTool.theta_grip_1 = randomNumber(stdev,mean);
	
	// lets a assign the udownside one 2, and set positive as clockwise
	angle = randomNumber(stdev,mean);
	newTool.theta_grip_2 += (angle/10.0)*1000.0;
	if (newTool.theta_grip_1 < -1.2*M_PI/2 || newTool.theta_grip_1 > 1.2*M_PI/2)   //use M_PI HERE
		newTool.theta_grip_1 = randomNumber(stdev,mean);

	///if the two joints get overflow/////
	if (newTool.theta_grip_1 > newTool.theta_grip_2)
		newTool.theta_grip_1 = newTool.theta_grip_2 - randomNumber(stdev,mean);



   /***********compute exponential map for forward kinematics**********/
	cv::Matx<double,3,3> roll_mat;
	cv::Matx<double,3,3> pitch_mat;
	cv::Matx<double,3,3> yaw_mat;

    roll_mat(0)(0) = 1;
    roll_mat(0)(1) = 0;
    roll_mat(0)(2) = 0;
    roll_mat(1)(0) = 0;
    roll_mat(1)(1) = cos(newTool.rvec(0));
    roll_mat(1)(2) = -sin(newTool.rvec(0));
    roll_mat(2)(0) = 0;
    roll_mat(2)(1) = sin(newTool.rvec(0));
    roll_mat(2)(2) = cos(newTool.rvec(0));

    pitch_mat(0)(0) = cos(newTool.rvec(1));
    pitch_mat(0)(1) = 0;
    pitch_mat(0)(2) = sin(newTool.rvec(1));
    pitch_mat(1)(0) = 0;
    pitch_mat(1)(1) = 1;
    pitch_mat(1)(2) = 0;
    pitch_mat(2)(0) = sin(newTool.rvec(1));
    pitch_mat(2)(1) = 0;
    pitch_mat(2)(2) = cos(newTool.rvec(1));

    yaw_mat(0)(0) = cos(newTool.rvec(2));
    yaw_mat(0)(1) = -sin(newTool.rvec(2));
    yaw_mat(0)(2) = 0;
    yaw_mat(1)(0) = sin(newTool.rvec(2));
    yaw_mat(1)(1) = cos(newTool.rvec(2));
    yaw_mat(1)(2) = 0;
    yaw_mat(2)(0) = 0;
    yaw_mat(2)(1) = 0;
    yaw_mat(2)(2) = 1;

    cv::Matx<double,3,3> rotation_mat;
    rotation_mat = yaw_mat * pitch_mat * roll_mat;
    
    //w is z of rotation mat
    
    newTool.w_z(0) = rotation_mat(0)(2);
    newTool.w_z(1) = rotation_mat(1)(2);
    newTool.w_z(2) = rotation_mat(2)(2);

    newTool.w_x(0) = rotation_mat(0)(0);
    newTool.w_x(1) = rotation_mat(1)(0);
    newTool.w_x(2) = rotation_mat(2)(0);
    /*get new tool model info*/



	return newTool;
}

cv::Rect ToolModel::renderTool(const toolModel &tool_struct, double stdev, double mean){
    cv::Matx<double,3,3> wz_mat;

	wz_mat(0)(0) = 0;
    wz_mat(0)(1) = -newTool.w_z(2);
    wz_mat(0)(2) = newTool.w_z(1);
    wz_mat(1)(0) = newTool.w_z(2);
    wz_mat(1)(1) = 0;
    wz_mat(1)(2) = -newTool.w_z(0);
    wz_mat(2)(0) = -newTool.w_z(1);
    wz_mat(2)(1) = newTool.w_z(0);
    wz_mat(2)(2) = 0;

	cv::Matx<double,3,3> exp_R;
    exp_R = I + wz_mat * sin(newTool.theta_ellipse) + wz_mat * wz_mat * (1-cos(newTool.theta_ellipse));




};