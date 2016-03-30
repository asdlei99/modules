##################################################
# application: get_proc_pid
# author : feng.yang
##################################################

cmake_minimum_required(VERSION 2.8.0)
project(getProcPid)

add_executable(get_proc_pid get_proc_pid.c)
install(TARGETS get_proc_pid DESTINATION bin)
