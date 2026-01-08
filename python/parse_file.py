import matplotlib.pyplot as plt
from struct import unpack
import radarprocessor
import cameraprocessor
import numpy as np

import cv2

if __name__ == "__main__":
    file_name = "data/pse84_wifi_streaming_4.log"

    radar_processor = radarprocessor.RadarProcessor()

    amp_over_time = []

    with open(file_name, mode='rb') as file: # b is important -> binary
        fileContent = file.read()

        camera_count = 0
        radar_count = 0

        # 1280 x 480 -> concatenation of the plot and of the video
        video=cv2.VideoWriter("video.avi",cv2.VideoWriter_fourcc(*'MJPG'),6.0,(1280,480))

        index = 0
        while True:
            packetType = fileContent[index]
            packetLen = int.from_bytes(fileContent[index+1:index+4], "little")

            # Check the type
            if (packetType == 0x76):
                camera_count+=1
            elif (packetType == 0x55):
                radar_count+=1
            else:
                print("Index: ", index)
                print("Packet type is not correct: ", packetType)
                break

            index += 5

            # Read the data
            raw_data = fileContent[index:index+packetLen]

            if (packetType == 0x55):
                # Convert to ushorts
                radar_data = unpack('H'*(len(raw_data)//2),raw_data)

                # Get maximum amplitude
                amp_over_time.append(radar_processor.get_max_doppler(radar_data))
            
            if (packetType == 0x76):
                # Convert to ushorts
                print('camera index:', camera_count)
                camera_data = unpack('H'*(len(raw_data)//2),raw_data)
                img = cameraprocessor.process_camera_data(camera_data)

                # Concatenate the radar values
                fig = plt.figure()
                plt.plot(amp_over_time)
                plt.grid(True)
                fig.canvas.draw()
                # converting matplotlib figure to Opencv image
                #plot = np.fromstring(fig.canvas.tostring_argb(), dtype=np.uint8,
                #                    sep='')
                plot = np.frombuffer(fig.canvas.tostring_argb(), dtype=np.uint8)
                
                r = plot[1::4]  # Red channel
                g = plot[2::4]  # Green channel
                b = plot[3::4]  # Blue channel
                plot = np.stack([r, g, b], axis=-1).reshape(fig.canvas.get_width_height()[::-1] + (3,))

                plot = cv2.cvtColor(plot, cv2.COLOR_RGB2BGR)
                resized = cv2.resize(img, (640,480))
                result_img = np.hstack([resized, plot])
                
                video.write(result_img)
                plt.close(fig)

            # Jump
            index += packetLen

            if (index >= len(fileContent)):
                break

        video.release()

        print("Found camera data: ", camera_count)
        print("Found radar data: ", radar_count)

        plt.figure()
        plt.title("Doppler magnitude over time")
        plt.plot(amp_over_time)
        plt.grid(True)
        plt.xlabel("Time (frame)")
        plt.ylabel("Magnitude")
        plt.show()
