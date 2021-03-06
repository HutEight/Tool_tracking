/*
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2016 Case Western Reserve University
 *    
 *    Orhan Ozguner <oxo31@case.edu>
 *    Russell Jackson <rcj33@case.edu>
 *    Ran Hao <rxh349@case.edu>
 *
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of Case Western Reserve University, nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef PARTICLEFILTER_H
#define PARTICLEFILTER_H

#include <vector>
#include <stdio.h>
#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <stdlib.h>
#include <math.h>

#include <string>
#include <cstring>

#include <tool_model_lib/tool_model.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <sensor_msgs/image_encodings.h>
#include <ros/ros.h>
#include <cv_bridge/cv_bridge.h>

#include <vesselness_image_filter_cpu/vesselness_lib.h>

class ParticleFilter {

private:

    cv::Mat Cam;
    ros::NodeHandle nh_;  //may need this

    ///ros::Timer timer;

    ToolModel newToolModel;  /// it should be set up the first time, probably need updates of the camera poses

    ToolModel::toolModel initial; //initial configuration

    unsigned int toolSize; //size of the needle to be rendered

    double perturbStd; //standard deviation for perturbing, if we obtain good matching it gets smaller to stay in the region

    unsigned int numParticles; //total number of particles
    cv::Mat toolImage_left; //left rendered Image
    cv::Mat toolImage_right; //right rendered Image


    cv::Rect ROI_left; //ROI for the left image
    cv::Rect ROI_right; //ROI for the right image

    std::vector<ToolModel::toolModel> particles; // particles
    std::vector<double> matchingScores; // particle scores (matching scores)
    std::vector<double> particleWeights; // particle weights calculated from matching scores


public:

    /*
    * The default constructor
    */
    ParticleFilter(ros::NodeHandle *nodehandle);

    /*
     * The deconstructor 
     */
    ~ParticleFilter();

    /*
    * The initializeParticles initializes the particles by setting the total number of particles, initial
    * guess and randomly generating the particles around the initial guess.
    */
    void initializeParticles();

    /***consider getting a timer to debug***/
    // void timerCallback(const ros::TimerEvent&);

    /*
     * This is the main function for tracking the needle. This function is called and it syncs all of the functions
    */
    std::vector<cv::Mat>
    trackingTool(const cv::Mat &bodyVel, const cv::Mat &segmented_left, const cv::Mat &segmented_right,
                 const cv::Mat &P_left, const cv::Mat &P_right);

    /*
     * resampling method
     */
    void
    resampleLowVariance(const std::vector<ToolModel::toolModel> &initial, const std::vector<double> &particleWeight,
                        std::vector<ToolModel::toolModel> &results);

    /*
     * update particles
     */
    void updateParticles(const cv::Mat &bodyVel, double &updateRate);

    /*
     * perturb the particles for more usable poses
     */
    cv::Mat addNoise(cv::Mat &inputMat);

    cv::Mat adjoint(cv::Mat &G);

};

#endif