/*
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2011, Willow Garage, Inc.
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
 *   * Neither the name of Willow Garage, Inc. nor the names of its
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
 *
 * Author: Suat Gedikli (gedikli@willowgarage.com)
 */

#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/io/pcd_io.h>
#include <pcl/io/openni_grabber.h>
#include <pcl/common/time.h>
#include <pcl/console/parse.h>

class SimpleOpenNIProcessor
{
  public:
    int waitTime; //Higher = fewer scans per second.
    int frame_counter;

    bool save;
    openni_wrapper::OpenNIDevice::DepthMode mode;

    SimpleOpenNIProcessor (openni_wrapper::OpenNIDevice::DepthMode depth_mode = openni_wrapper::OpenNIDevice::OpenNI_12_bit_depth) : mode (depth_mode), frame_counter(0) {}

    void cloud_cb_ (const pcl::PointCloud<pcl::PointXYZRGBA>::ConstPtr &cloud)
    {
      static unsigned count = 0;
      static double last = pcl::getTime ();
      if (++count == waitTime)
      {
        double now = pcl::getTime ();
        // std::cout << "distance of center pixel :" << cloud->points [(cloud->width >> 1) * (cloud->height + 1)].z << " mm. Average framerate: " << double(count)/double(now - last) << " Hz" <<  std::endl;
        count = 0;
        last = now;

        if (save)
        {
          //std::stringstream ss;
          //ss << std::setprecision (12) << pcl::getTime () * 100 << ".pcd";
          //ss << ++frame_counter << ".pcd";
          char label[255];
          sprintf(label, "%.4d.pcd", ++frame_counter);
          pcl::PCDWriter w;
          w.writeBinaryCompressed (label, *cloud);
          //std::cout << "\033[2;37mWrote point clouds to file: \033[2;36m" << ss.str () << "\033[0m" << std::endl;
        }
      }
    }

    // void
    // image_callback (const boost::shared_ptr<openni_wrapper::Image>& image)
    // {
    //   static unsigned count = 0;
    //   if (++count == waitTime)
    //   {
    //     image_ = image;
        
    //     if (image->getEncoding () != openni_wrapper::Image::RGB)
    //     {
    //       if (rgb_data_size_ < image->getWidth () * image->getHeight ())
    //       {
    //         if (rgb_data_)
    //           delete [] rgb_data_;
    //         rgb_data_size_ = image->getWidth () * image->getHeight ();
    //         rgb_data_ = new unsigned char [rgb_data_size_ * 3];
    //       }
    //       image_->fillRGB (image_->getWidth (), image_->getHeight (), rgb_data_);
    //     }

    //     if (save)
    //     {
    //       char label[255];
    //       sprintf(label, "%.4d.png", ++frame_counter);
    //       pcl::PCDWriter w;
    //       pcl::io::saveRgbPNGFile(label, rgb_data_, image->getWidth (), image->getHeight ());
    //     } 
    //   }
    // }


    void imageDepthImageCallback (const boost::shared_ptr<openni_wrapper::Image>&, const boost::shared_ptr<openni_wrapper::DepthImage>& d_img, float constant)
    {
      static unsigned count = 0;
      static double last = pcl::getTime ();
      if (++count == waitTime)
      {
        double now = pcl::getTime ();
        // std::cout << "got synchronized image x depth-image with constant factor: " << constant << ". Average framerate: " << double(count)/double(now - last) << " Hz" <<  std::endl;
        // std::cout << "Depth baseline: " << d_img->getBaseline () << " and focal length: " << d_img->getFocalLength () << std::endl;
        count = 0;
        last = now;
      }
    }

    // This is where the action is.
    void run ()
    {
      save = true;
      waitTime = 1; //Higher = fewer scans per second.
      
      // std::cout << "<Ctrl+C> to stop scanning." << std::endl;
      std::cout << std::endl;
      
      //----------------------Dont worry about this stuff -----------------------------
      pcl::OpenNIGrabber interface; // create a new grabber for OpenNI devices
      interface.getDevice ()->setDepthOutputFormat (mode); // Set the depth output format
      boost::function<void (const pcl::PointCloud<pcl::PointXYZRGBA>::ConstPtr&)> f = boost::bind (&SimpleOpenNIProcessor::cloud_cb_, this, _1); // make callback function from member function
      boost::signals2::connection c = interface.registerCallback (f); // connect callback function for desired signal. In this case its a point cloud with color values
      boost::function<void (const boost::shared_ptr<openni_wrapper::Image>&, const boost::shared_ptr<openni_wrapper::DepthImage>&, float constant)> f2 = boost::bind (&SimpleOpenNIProcessor::imageDepthImageCallback, this, _1, _2, _3); // make callback function from member function
      boost::signals2::connection c2 = interface.registerCallback (f2); // connect callback function for desired signal. In this case its a point cloud with color values
      //--------------------------------------------------------------------------------

      // start receiving point clouds
      interface.start ();
      while(true){}
    }
};

int main (int argc, char **argv)
{
  int mode = openni_wrapper::OpenNIDevice::OpenNI_12_bit_depth;
  pcl::console::parse_argument (argc, argv, "-mode", mode);

  SimpleOpenNIProcessor v (static_cast<openni_wrapper::OpenNIDevice::DepthMode> (mode));
  v.run ();
  return (0);
}