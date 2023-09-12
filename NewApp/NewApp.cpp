#include <opencv2/opencv.hpp>
#include <nuitrack/Nuitrack.h>
#include <signal.h>
#include <iostream>
#include <array>
#include <vector>

using namespace tdv::nuitrack;
using namespace tdv::nuitrack::device;
using namespace cv;

int fps;
bool finished;

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

void signalHandler(int signal) {
	if (signal == SIGINT)
    {
		finished = true;
}
}



void onSkeletonUpdate(tdv::nuitrack::SkeletonData::Ptr skeletonData)
{
    // Create an empty OpenCV Mat to display skeleton
    cv::Mat skeletonImage = cv::Mat::zeros(480, 640, CV_8UC3);  // Assuming VGA resolution. Change if needed.

    const auto& skeletons = skeletonData->getSkeletons();

    for (const auto& skeleton : skeletons)
    {
        const auto& joints = skeleton.joints;

        for (const auto& joint : joints)
        {
            int x = joint.proj.x * 640;  // Assuming VGA resolution
            int y = joint.proj.y * 480;  // Assuming VGA resolution

            // Draw the joint as a circle
            cv::circle(skeletonImage, cv::Point(x, y), 5, cv::Scalar(0, 255, 0), -1);
        }
    }

    // Show the image
    cv::imshow("Skeleton", skeletonImage);
    cv::waitKey(1);
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


int main() {
    	tdv::nuitrack::Nuitrack::init();
	 	const NuitrackDevice::Ptr device = selectDevice(); 
        selectDeviceVideoMode(device, StreamType::DEPTH);
		selectDeviceVideoMode(device, StreamType::COLOR);
		
		tdv::nuitrack::Nuitrack::setDevice(device);
    // Create instances
    	auto colorSensor = ColorSensor::create();
    


		tdv::nuitrack::SkeletonTracker::Ptr skeletonTracker = tdv::nuitrack::SkeletonTracker::create();

    // Connect the onNewFrame handler to the SkeletonTracker
    	skeletonTracker->connectOnUpdate(&onSkeletonUpdate);

    // Run
   	  tdv::nuitrack::Nuitrack::run();
	  signal(SIGINT, &signalHandler);
    // Main loop
    while (!finished) {

		tdv::nuitrack::Nuitrack::waitUpdate(skeletonTracker);
		
       // tdv::nuitrack::Nuitrack::waitUpdate(colorSensor);
		//waitKey(1000/fps);
    }

    // Release all
    tdv::nuitrack::Nuitrack::release();
    return 0;
}
