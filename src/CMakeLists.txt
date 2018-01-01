SET(PTHREADPOOL lxl_main.c lxl_log.c lxl_misc.c lxl_tpl.c lxl_thread_queue.c)
ADD_EXECUTABLE(main ${PTHREADPOOL})
TARGET_LINK_LIBRARIES(main pthread)

