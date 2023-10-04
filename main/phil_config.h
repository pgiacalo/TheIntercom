#ifndef CONFIG_H
#define CONFIG_H

//PHIL added this config file to set the role and the mac addresses

// Define your settings here
#define ROLE_MASTER     0
#define ROLE_SLAVE      1
#define DEVICE_ROLE     ROLE_SLAVE  // You can change this as needed

#if DEVICE_ROLE == ROLE_MASTER
#define HF_PEER_ADDR {0x58, 0xFC, 0xC6, 0x6C, 0x0A,0x03}
#elif DEVICE_ROLE == ROLE_SLAVE
#define HF_PEER_ADDR {0x54, 0xB7, 0xE5, 0x8C, 0x07,0x71}
#endif

#endif // CONFIG_H
