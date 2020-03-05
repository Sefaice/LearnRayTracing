// glad
#include <glad/glad.h>
// glfw3
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

// IMGUI
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include "./shader.h"
#include "./maths.h"
#include "./utils.h"

#include <iostream>
#include <vector>
#include <stdlib.h>

const unsigned int SCR_WIDTH = 256;
const unsigned int SCR_HEIGHT = 256;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

void renderQuad();

float deltaTime = 0.0f;	
float lastFrame = 0.0f;

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "RayTracing", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// first color framebuffer
	unsigned int framebuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	// color texture
	unsigned int textureColorbuffer;
	glGenTextures(1, &textureColorbuffer);
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
	// normal texture
	unsigned int textureNormalbuffer;
	glGenTextures(1, &textureNormalbuffer);
	glBindTexture(GL_TEXTURE_2D, textureNormalbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, textureNormalbuffer, 0);
	// world pos
	unsigned int textureWorldPosbuffer;
	glGenTextures(1, &textureWorldPosbuffer);
	glBindTexture(GL_TEXTURE_2D, textureWorldPosbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, textureWorldPosbuffer, 0);
	// texture color
	unsigned int textureTexbuffer;
	glGenTextures(1, &textureTexbuffer);
	glBindTexture(GL_TEXTURE_2D, textureTexbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, textureTexbuffer, 0);
	// color std var
	unsigned int texture_color_stdvar_buffer;
	glGenTextures(1, &texture_color_stdvar_buffer);
	glBindTexture(GL_TEXTURE_2D, texture_color_stdvar_buffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, texture_color_stdvar_buffer, 0);
	// normal std var
	unsigned int texture_normal_stdvar_buffer;
	glGenTextures(1, &texture_normal_stdvar_buffer);
	glBindTexture(GL_TEXTURE_2D, texture_normal_stdvar_buffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT5, GL_TEXTURE_2D, texture_normal_stdvar_buffer, 0);
	// world pos std var
	unsigned int texture_worldpos_stdvar_buffer;
	glGenTextures(1, &texture_worldpos_stdvar_buffer);
	glBindTexture(GL_TEXTURE_2D, texture_worldpos_stdvar_buffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT6, GL_TEXTURE_2D, texture_worldpos_stdvar_buffer, 0);
	// - tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
	unsigned int attachments[7] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, 
		GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5,
		GL_COLOR_ATTACHMENT6 };
	glDrawBuffers(7, attachments);

	// second color framebuffer
	unsigned int lastFramebuffer;
	glGenFramebuffers(1, &lastFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, lastFramebuffer);
	unsigned int lastTextureColorbuffer;
	glGenTextures(1, &lastTextureColorbuffer);
	glBindTexture(GL_TEXTURE_2D, lastTextureColorbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lastTextureColorbuffer, 0);
	// normal texture
	unsigned int lastTextureNormalbuffer;
	glGenTextures(1, &lastTextureNormalbuffer);
	glBindTexture(GL_TEXTURE_2D, lastTextureNormalbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, lastTextureNormalbuffer, 0);
	// world pos
	unsigned int lastTextureWorldPosbuffer;
	glGenTextures(1, &lastTextureWorldPosbuffer);
	glBindTexture(GL_TEXTURE_2D, lastTextureWorldPosbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, lastTextureWorldPosbuffer, 0);
	// texture color
	unsigned int lastTextureTexbuffer;
	glGenTextures(1, &lastTextureTexbuffer);
	glBindTexture(GL_TEXTURE_2D, lastTextureTexbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, lastTextureTexbuffer, 0);
	// color std var
	unsigned int lasttexture_color_stdvar_buffer;
	glGenTextures(1, &lasttexture_color_stdvar_buffer);
	glBindTexture(GL_TEXTURE_2D, lasttexture_color_stdvar_buffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, lasttexture_color_stdvar_buffer, 0);
	// normal std var
	unsigned int lasttexture_normal_stdvar_buffer;
	glGenTextures(1, &lasttexture_normal_stdvar_buffer);
	glBindTexture(GL_TEXTURE_2D, lasttexture_normal_stdvar_buffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT5, GL_TEXTURE_2D, lasttexture_normal_stdvar_buffer, 0);
	// world pos std var
	unsigned int lasttexture_worldpos_stdvar_buffer;
	glGenTextures(1, &lasttexture_worldpos_stdvar_buffer);
	glBindTexture(GL_TEXTURE_2D, lasttexture_worldpos_stdvar_buffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT6, GL_TEXTURE_2D, lasttexture_worldpos_stdvar_buffer, 0);
	// - tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
	glDrawBuffers(7, attachments);
	
	Shader myShader("../../src/gpu/vertexShader.vs.glsl", "../../src/gpu/fragmentShader.fs.glsl");
	Shader quadShader("../../src/gpu/quadShader.vs.glsl", "../../src/gpu/quadShader.fs.glsl");

	glm::vec3 lookfrom = glm::vec3(0, 2, 2.5);
	glm::vec3 lookat = glm::vec3(-2, 0, 0);
	float distToFocus = 3.0f;
	float aperture = 0.05f;
	Camera cam(lookfrom, lookat, glm::vec3(0, 1, 0), 60, float(SCR_WIDTH) / float(SCR_HEIGHT), aperture, distToFocus);

	myShader.use();
	myShader.setInt("LastColorTexture", 0);
	myShader.setInt("LastNormalTexture", 1);
	myShader.setInt("LastWorldPosTexture", 2);
	myShader.setInt("LastTexTexture", 3);
	myShader.setInt("LastColorStdvarTexture", 4);
	myShader.setInt("LastNormalStdvarTexture", 5);
	myShader.setInt("LastWorldPosStdvarTexture", 6);
	quadShader.use();
	quadShader.setInt("ColorTexture", 0);

	int frameCount = 0;

	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		// show FPS
		int fps = 1 / deltaTime;
		//glfwSetWindowTitle(window, ("FPS: " + std::to_string(fps)).c_str());
		glfwSetWindowTitle(window, ("Frame Count: " + std::to_string(frameCount)).c_str());

		processInput(window);

		// render into switched framebuffer
		int switchBuffer = frameCount % 2;
		glBindFramebuffer(GL_FRAMEBUFFER, switchBuffer == 0 ? framebuffer : lastFramebuffer);
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, switchBuffer == 0 ? lastTextureColorbuffer : textureColorbuffer);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, switchBuffer == 0 ? lastTextureNormalbuffer : textureNormalbuffer);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, switchBuffer == 0 ? lastTextureWorldPosbuffer : textureWorldPosbuffer);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, switchBuffer == 0 ? lastTextureTexbuffer : textureTexbuffer);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, switchBuffer == 0 ? lasttexture_color_stdvar_buffer : texture_color_stdvar_buffer);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, switchBuffer == 0 ? lasttexture_normal_stdvar_buffer : texture_normal_stdvar_buffer);
		glActiveTexture(GL_TEXTURE6);
		glBindTexture(GL_TEXTURE_2D, switchBuffer == 0 ? lasttexture_worldpos_stdvar_buffer : texture_worldpos_stdvar_buffer);
		// render
		myShader.use();
		myShader.setVec3("origin", cam.origin);
		myShader.setVec3("lowerLeftCorner", cam.lowerLeftCorner);
		myShader.setVec3("horizontalVec", cam.horizontalVec);
		myShader.setVec3("verticalVec", cam.verticalVec);
		myShader.setVec3("u", cam.u);
		myShader.setVec3("v", cam.v);
		myShader.setFloat("lensRadius", cam.lensRadius);
		myShader.setInt("frameCount", frameCount);
		myShader.setFloat("time", glfwGetTime());
		myShader.setInt("SCR_WIDTH", SCR_WIDTH);
		myShader.setInt("SCR_HEIGHT", SCR_HEIGHT);
		renderQuad();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		quadShader.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, switchBuffer == 0 ? textureColorbuffer : lastTextureColorbuffer);
		renderQuad();

		frameCount++;

		// save {2, 4, 8, 16...} spp image
		if ((frameCount & frameCount - 1) == 0) {
		/*if (frameCount == 4 || frameCount == 8192) {*/
			// color
			glPixelStorei(GL_PACK_ALIGNMENT, 1);
			uint8_t* raw_img = (uint8_t*)malloc(sizeof(uint8_t) * SCR_WIDTH * SCR_HEIGHT * 3);
			glBindTexture(GL_TEXTURE_2D, switchBuffer == 0 ? textureColorbuffer : lastTextureColorbuffer); // bind matched texturebuffer
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, raw_img);
			saveTextureToBMP(SCR_WIDTH, SCR_HEIGHT, raw_img,
				("../../src/gpu_out/color_spp_" + std::to_string(frameCount) + ".bmp").c_str());
			// file
			glPixelStorei(GL_PACK_ALIGNMENT, 1);
			uint8_t* uint8Data = (uint8_t*)malloc(sizeof(uint8_t) * SCR_WIDTH * SCR_HEIGHT * 3);
			glBindTexture(GL_TEXTURE_2D, switchBuffer == 0 ? textureColorbuffer : lastTextureColorbuffer); // bind matched texturebuffer
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, uint8Data);
			saveTextureToBinary_uint8(SCR_WIDTH, SCR_HEIGHT, uint8Data,
				("../../src/gpu_out/color_spp_" + std::to_string(frameCount) + ".fgg").c_str());
			// texture color
			glPixelStorei(GL_PACK_ALIGNMENT, 1);
			raw_img = (uint8_t*)malloc(sizeof(uint8_t) * SCR_WIDTH * SCR_HEIGHT * 3);
			glBindTexture(GL_TEXTURE_2D, switchBuffer == 0 ? textureTexbuffer : lastTextureTexbuffer); // bind matched texturebuffer
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, raw_img);
			saveTextureToBMP(SCR_WIDTH, SCR_HEIGHT, raw_img,
				("../../src/gpu_out/texture_spp_" + std::to_string(frameCount) + ".bmp").c_str());
		}
		if (frameCount == 4) {
			// normal
			float* floatData = (float*)malloc(sizeof(float) * SCR_WIDTH * SCR_HEIGHT * 3);
			glBindTexture(GL_TEXTURE_2D, switchBuffer == 0 ? textureNormalbuffer : lastTextureNormalbuffer);
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, floatData);
			saveTextureToBinary_float(SCR_WIDTH, SCR_HEIGHT, floatData,
				("../../src/gpu_out/normal_spp_" + std::to_string(frameCount) + ".fgg").c_str());
			// world pos
			glBindTexture(GL_TEXTURE_2D, switchBuffer == 0 ? textureWorldPosbuffer : lastTextureWorldPosbuffer);
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, floatData);
			saveTextureToBinary_float(SCR_WIDTH, SCR_HEIGHT, floatData,
				("../../src/gpu_out/worldpos_spp_" + std::to_string(frameCount) + ".fgg").c_str());
			// color std var
			glBindTexture(GL_TEXTURE_2D, switchBuffer == 0 ? texture_color_stdvar_buffer : lasttexture_color_stdvar_buffer);
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, floatData);
			saveTextureToBinary_float(SCR_WIDTH, SCR_HEIGHT, floatData,
				("../../src/gpu_out/color_stdvar_spp_" + std::to_string(frameCount) + ".fgg").c_str());
			// normal std var
			glBindTexture(GL_TEXTURE_2D, switchBuffer == 0 ? texture_normal_stdvar_buffer : lasttexture_normal_stdvar_buffer);
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, floatData);
			saveTextureToBinary_float(SCR_WIDTH, SCR_HEIGHT, floatData,
				("../../src/gpu_out/normal_stdvar_spp_" + std::to_string(frameCount) + ".fgg").c_str());
			// worldpos std var
			glBindTexture(GL_TEXTURE_2D, switchBuffer == 0 ? texture_worldpos_stdvar_buffer : lasttexture_worldpos_stdvar_buffer);
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, floatData);
			saveTextureToBinary_float(SCR_WIDTH, SCR_HEIGHT, floatData,
				("../../src/gpu_out/worldpos_stdvar_spp_" + std::to_string(frameCount) + ".fgg").c_str());
		}

		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	glfwTerminate();

	system("Pause");
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

//unsigned int quadVAO = 0;
//unsigned int quadVBO = 0;
//void renderQuad()
//{
//	if (quadVAO == 0)
//	{
//		float quadVertices[] = {
//			-1.0f,  1.0f,
//			-1.0f, -1.0f,
//			1.0f,  1.0f,
//			1.0f, -1.0f,
//		};
//		glGenVertexArrays(1, &quadVAO);
//		glGenBuffers(1, &quadVBO);
//		glBindVertexArray(quadVAO);
//		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
//		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
//		glEnableVertexAttribArray(0);
//		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
//	}
//
//	glBindVertexArray(quadVAO);
//	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
//	glBindVertexArray(0);
//}

unsigned int quadVAO = 0;
unsigned int quadVBO = 0;
void renderQuad()
{
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}