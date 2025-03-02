#include "mount_daemon.h"

int main(){
  Mount_Daemon mount_daemon;
  mount_daemon.init("vohnok", false);
  mount_daemon.start();
  while (true){
    printf(".");
    usleep(250*1000);
  }
}