##################################################
# application: check net
# author : feng.yang
##################################################

cmake_minimum_required(VERSION 2.8.0)
project(check_net)

add_executable(checknet check_net.c)
install(TARGETS checknet DESTINATION bin)
