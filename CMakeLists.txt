cmake_minimum_required(VERSION 2.8)

project(nlimagemaker C)
add_executable(${PROJECT_NAME} "main.c")

INSTALL(TARGETS nlimagemaker
	RUNTIME DESTINATION sbin
)
