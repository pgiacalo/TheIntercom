idf_component_register(SRCS "app_hf_msg_prs.c"
                            "app_hf_msg_set.c"
                            "bt_app_core.c"
                            "bt_app_hf.c"
                            "gpio_pcm_config.c"
                            "main.c"
                    INCLUDE_DIRS ".")

# //PHIL 
# Modify this CMakeLists.txt file, as follows, to override code found in the esp-idf library
# set(SOURCES 
#     "main.c"
#     "components/btc_hf_client.c"  # Your modified file's path relative to the main directory
#     ...
# )
# 
# idf_component_register(
#     SRCS ${SOURCES}
#     EXCLUDE_SRCS "${IDF_PATH}/components/bt/host/bluedroid/btc/profile/std/hf_client/btc_hf_client.c"  # original file's path under IDF_PATH
#     INCLUDE_DIRS "."
# )
