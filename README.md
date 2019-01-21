**Volume Service**

Volume service detect and count vehicle from video, you need to have camera-service and traffic-streaming built to run this.
 - camera-service: https://github.com/hanalaydrus/camera-service
 - traffic-streaming: https://github.com/hanalaydrus/traffic-streaming

This are steps to run this code in your local:

**1. Install Docker**

 - Windows : https://docs.docker.com/docker-for-windows/install/ 

 - Mac : https://docs.docker.com/docker-for-mac/install/

 - Ubuntu : https://docs.docker.com/install/linux/docker-ce/ubuntu/

**2. Make sure docker-compose installed**, If not please check the link below
https://docs.docker.com/compose/install/

**3. Clone this repo** `git clone https://github.com/hanalaydrus/volume-of-vehicles-cpp.git`

**4. Run** `docker-compose up --build` **or** `sudo docker-compose up --build`

**5. The camera service should already run**, you could check it through `docker ps`
