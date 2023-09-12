
#include <opencv2/opencv.hpp>
#include <nuitrack/Nuitrack.h>
#include <signal.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <array>


using namespace tdv::nuitrack::device;
using namespace cv;


int fps;
namespace console {
	

	template<std::size_t Columns>
	class Table {
		using RowType = std::array<std::string, Columns>;

	public:
		Table(RowType&& header): _header(header) {
			for (std::size_t i = 0; i < Columns; ++i)
				_maxColumnLengths[i] = _header[i].length();
		}

		void addRow(RowType&& row) {
			_updateColumnLengths(row);
			_rows.push_back(std::move(row));
		}

		

	private:
		void _updateColumnLengths(const RowType& row) {
			for (std::size_t i = 0; i < Columns; ++i) {
				const auto length = row[i].length();
				if (length > _maxColumnLengths[i])
					_maxColumnLengths[i] = length;
			}
		}


	private:
		const RowType _header;
		std::vector<RowType> _rows;
		std::array<std::size_t, Columns> _maxColumnLengths;
	};

} // namespace console




// Function to convert tdv::nuitrack::Color3 to cv::Mat
cv::Mat convertColor3ToMat(const tdv::nuitrack::Color3* data, int rows, int cols) {
    cv::Mat mat(rows, cols, CV_8UC3);

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            int index = i * cols + j;
            mat.at<cv::Vec3b>(i, j) = cv::Vec3b(data[index].red, data[index].green, data[index].blue);
        }
    }

    return mat;
}



bool finished;
void signalHandler(int signal) {
	if (signal == SIGINT)
    {
		finished = true;
}
}

void depthfun(tdv::nuitrack::DepthFrame::Ptr depthframe)
{
    

    // Check if the depth frame is valid before displaying it
    if (depthframe->getCols() > 0 && depthframe->getRows() > 0) {
        // Process and display depth frame if needed
    } else {
        std::cerr << "Error: Invalid depth frame dimensions." << std::endl;
    }

    
}

void colorfun(tdv::nuitrack::RGBFrame::Ptr colorframe)
{


    // Convert the data to OpenCV format
    cv::Mat colorImage = convertColor3ToMat(colorframe->getData(), colorframe->getRows(), colorframe->getCols());

	
    // Check if the colorImage is valid before displaying it
    if (colorImage.cols > 0 && colorImage.rows > 0) {
		 cv::Mat rgbImage;
    	 cv::cvtColor(colorImage, rgbImage, cv::COLOR_BGR2RGB);
         cv::imshow("Color Viewer", rgbImage);
    } else {
        std::cerr << "Error: Invalid color image dimensions." << std::endl;
    }

	if (waitKey(1000 / fps) >= 0)
	{
	}
}





void selectDeviceVideoMode(NuitrackDevice::Ptr device, StreamType streamType) {
	std::string streamName;
	switch (streamType) {
		case StreamType::DEPTH:
			streamName = "depth"; break;
		case StreamType::COLOR:
			streamName = "color"; break;
	}

	const auto videoModes = device->getAvailableVideoModes(streamType);
	const auto vmSize = videoModes.size();

	console::Table<4> table({"Index", "Width", "Height", "FPS"});
	for (std::size_t i = 0; i < vmSize; ++i) {
		const auto vmWidth = videoModes[i].width;
		const auto vmHeight = videoModes[i].height;
		const auto vmFps = videoModes[i].fps;
		fps = vmFps;
		table.addRow({std::to_string(i), std::to_string(vmWidth), std::to_string(vmHeight), std::to_string(vmFps)});
	}

	
    
	
		device->setVideoMode(streamType, videoModes[- 1]);
}



NuitrackDevice::Ptr selectDevice() {
		std::vector<NuitrackDevice::Ptr> devices;
		while (true)
		{
			devices = tdv::nuitrack::Nuitrack::getDeviceList();
			
			if (!devices.empty())
				break;

			std::cout << "\nConnect sensor and press Enter to continue" << std::endl;
			std::cin.ignore();
		}

		console::Table<4> table({"Index", "Name", "Serial Number", "License"});
		for (std::size_t i = 0; i < devices.size(); i++)
		{
			const auto& device = devices[i];
			table.addRow({
				std::to_string(i),
				device->getInfo(DeviceInfoType::DEVICE_NAME),
				device->getInfo(DeviceInfoType::SERIAL_NUMBER),
				//device->getActivationStatus()
			});
		}
	;

		int devIndex = 0;

		return devices[devIndex];
}



int main(int argc, char* argv[]) {


 
        // Initialize Nuitrack
        tdv::nuitrack::Nuitrack::init();

       const NuitrackDevice::Ptr device = selectDevice(); 
        selectDeviceVideoMode(device, StreamType::DEPTH);
		selectDeviceVideoMode(device, StreamType::COLOR);
           try {
    
        

        try {
    // Set the device
        tdv::nuitrack::Nuitrack::setDevice(device);
    
} catch (const tdv::nuitrack::Exception& e) {
    // Handle the exception here (e.g., log the error, exit gracefully)
    std::cerr << "Exception when setting the device: " << e.what() << std::endl;
    return 1; // Exit or return an error code
}





       // tdv::nuitrack::DepthSensor::Ptr depthSensor = tdv::nuitrack::DepthSensor::create();
        // Create a ColorSensor
        tdv::nuitrack::ColorSensor::Ptr colorSensor = tdv::nuitrack::ColorSensor::create();
      



        

       // depthSensor->connectOnNewFrame(depthfun);
        colorSensor->connectOnNewFrame(colorfun);
        tdv::nuitrack::Nuitrack::run();
        // Main loop
        signal(SIGINT, &signalHandler);
        // Main loop for color frames
while (!finished) {
    // Wait for a new color frame
   
    // tdv::nuitrack::Nuitrack::waitUpdate(depthSensor);
     tdv::nuitrack::Nuitrack::waitUpdate(colorSensor);

    

    }
        // Release resources
        cv::destroyAllWindows();
        tdv::nuitrack::Nuitrack::release();
    
    }
    catch (const std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}


