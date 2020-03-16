#include <iostream>       
#include "..\..\include\interfacedevicestream.hpp"
#include <dlfcn.h>
#include <thread>
#include <chrono> 

class IPCContainer : public InterfaceMsgContainer
{
public:
    virtual void recvVideoBlob(const PureVideoBlob *video, const SmartDataBlob *data)
    {
      static int idx = 0;
      std::cout << idx++ << " " << video->height << "x"  << video->width << " "  << video->size << std::endl;
      std::cout << "    " << data->size << std::endl;
    }
    virtual void recvCommnBlob(const SmartDataBlob *data)
    {
      static int idx = 0;
      std::cout << idx++ << " " << data->size << std::endl;
    }

};

int main() 
{
  void* handle = dlopen("src\\freader.dll", RTLD_LAZY);

  CreateDeviceStream creator = (CreateDeviceStream)dlsym(handle, "CreateDeviceStream");
  DestroyDeviceStream destroyer = (DestroyDeviceStream)dlsym(handle, "DestroyDeviceStream");

  InterfaceDeviceStream *reader = creator();

  reader->setReceiver(new IPCContainer);
  reader->start();

  //reader keeps running 10 seconds.
  std::this_thread::sleep_for(std::chrono::seconds(10));
  reader->stop();
  destroyer(reader);

  dlclose(handle);
  return 0;
}