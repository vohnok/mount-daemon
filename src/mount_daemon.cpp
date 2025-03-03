#include "mount_daemon.h"

namespace fs = std::filesystem;

Mount_Daemon::Mount_Daemon(){
  is_daemon_mode = false;
  is_running = false;
}

Mount_Daemon::~Mount_Daemon(){
  if (is_running.load()) stop();
}

void Mount_Daemon::init(const std::string& second_logname, bool is_daemon_mode){
  reinit(second_logname, is_daemon_mode);
}

void Mount_Daemon::reinit(const std::string& second_logname, bool is_daemon_mode){
  this->logname = getenv("LOGNAME");
  if (logname.compare("root") == 0) logname = second_logname;
  this->mount_root = "/media/" + logname + "/";
  this->is_daemon_mode = is_daemon_mode;
}

void Mount_Daemon::start(){
  if (is_daemon_mode){
    if (daemon(0, 0) == -1){
      printf("Error while daemon: %s!\n", strerror(errno)); 
    }
    else printf("run in daemon mode!\n\n");
  }

  udev = udev_new();
  if (!udev) {
    printf("Can't create udev\n");
  }
   
  mon = udev_monitor_new_from_netlink(udev, "udev");
  assert(mon != NULL);

  assert(udev_monitor_filter_add_match_subsystem_devtype(mon, "block", NULL) >=0);
  assert(udev_monitor_filter_add_match_subsystem_devtype(mon, "usb","usb-device") >=0);
  udev_monitor_enable_receiving(mon);

  fd = udev_monitor_get_fd(mon);
  if (fd > 0){
    is_running = true;
  
    main_thread = std::thread(&Mount_Daemon::main_task, this);  
  }
  else{
    printf("Error while open file descriptor of udev monitor: %s\n", strerror(errno));
  }
}

void Mount_Daemon::stop(){
  is_running = false;

  if (main_thread.joinable())
    main_thread.join();
}

void Mount_Daemon::restart(){
  if (is_running.load()) stop();
  
  start();
}

void Mount_Daemon::main_task(void){
  fd_set fds;
  struct timeval tv;
  int ret;

  while (is_running.load()) 
  {
    FD_ZERO(&fds);
    FD_SET(fd, &fds);

    tv.tv_sec = 0;
    tv.tv_usec = 0;

    ret = select(fd+1, &fds, NULL, NULL, &tv);

    if (ret > 0 && FD_ISSET(fd, &fds)) {
      dev = udev_monitor_receive_device(mon);
      if (dev) {
        std::string devtype=udev_device_get_devtype(dev) != NULL ? udev_device_get_devtype(dev) : "";
        std::string action=udev_device_get_action(dev) != NULL ? udev_device_get_action(dev) : "";
        std::string devnode=udev_device_get_devnode(dev) != NULL ? udev_device_get_devnode(dev) : "";
        std::string subsystem=udev_device_get_subsystem(dev) != NULL ? udev_device_get_subsystem(dev) : "";
        std::string id_fs_type=udev_device_get_property_value(dev, "ID_FS_TYPE") != NULL ? udev_device_get_property_value(dev, "ID_FS_TYPE") : "";
        std::string id_fs_label=udev_device_get_property_value(dev, "ID_FS_LABEL") != NULL ? udev_device_get_property_value(dev, "ID_FS_LABEL") : "";
        std::string sysname=udev_device_get_sysname(dev) != NULL ? udev_device_get_sysname(dev) : "";
        std::string id_pat_entry_type=udev_device_get_property_value(dev, "ID_PART_ENTRY_TYPE") != NULL ? udev_device_get_property_value(dev, "ID_PART_ENTRY_TYPE") : "";

#ifdef __DEBUG__
        printf("\n\nNode: %s\n"          , devnode    .c_str());
        printf("   Subsystem: %s\n"      , subsystem  .c_str());
        printf("   sysname: %s\n",udev_device_get_sysname(dev));
        printf("   syspath:  %s\n", udev_device_get_syspath(dev));
        printf("   subsystem:  %s\n", udev_device_get_subsystem(dev));
        printf("   sysnum:  %s\n", udev_device_get_sysnum(dev));
        printf("   Devtype: %s\n"        , devtype    .c_str());
        printf("   Action: %s\n"         , action     .c_str());
        printf("   ID_FS_TYPE: %s\n"     , id_fs_type .c_str());
        printf("   ID_PART_ENTRY_TYPE: %s\n"     , udev_device_get_property_value(dev, "ID_PART_ENTRY_TYPE"));
        printf("   DRIVER: %s\n"     , udev_device_get_property_value(dev, "DRIVER"));
        printf("   ID_FS_LABEL: %s\n\n\n", id_fs_label.c_str());
#endif
        if (id_fs_label.length() == 0) id_fs_label = sysname;

        if (devtype.compare("partition")==0 && action.compare("add") == 0 && subsystem.compare("block") == 0 && id_pat_entry_type.compare("c12a7328-f81f-11d2-ba4b-00a0c93ec93b") != 0 && id_pat_entry_type.compare("e3c9e316-0b5c-4db8-817d-f92df00215ae") != 0 && id_pat_entry_type.compare("de94bba4-06d1-4d40-a16a-bfd50179d6ac")){
          std::string mount_flags = id_fs_type.compare("ext4") == 0 ? "" : "iocharset=utf8";
          std::string dir_name =  mount_root + id_fs_label;         
#ifdef __DEBUG__
          printf("dir_name: %s\n", dir_name.c_str());
#endif
          fs::create_directory(dir_name);
          mount(devnode.c_str(), dir_name.c_str(), id_fs_type.c_str(), 0, mount_flags.c_str());
      }
      if (devtype.compare("partition")==0 && action.compare("remove") == 0 && subsystem.compare("block") == 0 && id_pat_entry_type.compare("c12a7328-f81f-11d2-ba4b-00a0c93ec93b") != 0 && id_pat_entry_type.compare("e3c9e316-0b5c-4db8-817d-f92df00215ae") != 0 && id_pat_entry_type.compare("de94bba4-06d1-4d40-a16a-bfd50179d6ac")){
          std::string dir_name =  mount_root + id_fs_label;    
#ifdef __DEBUG__
          printf("dir_name : %s\n", dir_name.c_str());
#endif
          umount((mount_root + id_fs_label).c_str());
          usleep(50*1000);
          fs::remove_all(dir_name);
        }

        udev_device_unref(dev);
      }
      else {
#ifdef __DEBUG__
        printf("No Device from receive_device(). An error occured.\n");
#endif
      }                   
    }
    usleep(50*1000);
    fflush(stdout);
  }
}