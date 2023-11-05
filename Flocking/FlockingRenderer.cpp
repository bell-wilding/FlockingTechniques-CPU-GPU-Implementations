#include "FlockingRenderer.h"
#include "Flock.h"
#include "Agent.h"
#include "../Common/Camera.h"
#include "../Common/Vector2.h"
#include "../Common/Vector3.h"
#include "../Common/TextureLoader.h"
using namespace NCL;
using namespace Rendering;

FlockingRenderer::FlockingRenderer() : OGLRenderer(*Window::GetWindow()) {
	glEnable(GL_DEPTH_TEST);

	numAgents = 0;
	
	darkMode = false;
	backgroundCol = Vector4(1, 1, 1, 1);
	boxCol = Vector4(0, 0, 0, 0);

	lightCol = Vector4(0.8f, 0.8f, 0.5f, 1.0f);
	lightPosition = Vector3(-200.0f, 60.0f, -200.0f);
	useLighting = true;

	visualiseGridColours = false;

	cam = new Camera();
	InitCamera();

	SetVerticalSync(VerticalSyncState::VSync_OFF);

	agentShader = new OGLShader("AgentVert.glsl", "AgentFrag.glsl");

	agentMesh = new OGLMesh("agent.msh");
	agentMesh->SetPrimitiveType(GeometryPrimitive::Triangles);
	agentMesh->UploadToGPU();

	glClearColor(backgroundCol.x, backgroundCol.y, backgroundCol.z, backgroundCol.w);
}

FlockingRenderer::~FlockingRenderer() {
	glDeleteBuffers(1, &bufFlock);

	delete agentMesh;
	delete agentShader;
}

void FlockingRenderer::InitCamera() {
	cam->SetPitch(-15.0f);
	cam->SetYaw(315.0f);
	cam->SetPosition(Vector3(-140, 40, 140));
}

void NCL::FlockingRenderer::InitFlock(GLuint bufFlock, int numAgents, float maxBound, float modelScale) {
	this->bufFlock = bufFlock;
	this->numAgents = numAgents;
	this->maxBound = maxBound;
	this->modelScale = modelScale;
}

void NCL::FlockingRenderer::ToggleLighting() {
	useLighting = !useLighting;
}

void NCL::FlockingRenderer::ToggleDarkMode() {
	darkMode = !darkMode;
	if (darkMode) {
		backgroundCol = Vector4(0, 0, 0, 0);
		boxCol = Vector4(1, 1, 1, 1);
	}
	else {
		backgroundCol = Vector4(1, 1, 1, 1);
		boxCol = Vector4(0, 0, 0, 0);
	}
}

void NCL::FlockingRenderer::ToggleGridVisualisation() {
	visualiseGridColours = !visualiseGridColours;
}

void NCL::FlockingRenderer::DrawBoundingBox(float bound) {
	DrawLine(Vector3(1, 1, 1) * bound, Vector3(1, 1, -1) * bound, boxCol);
	DrawLine(Vector3(1, 1, -1) * bound, Vector3(-1, 1, -1) * bound, boxCol);
	DrawLine(Vector3(-1, 1, -1) * bound, Vector3(-1, 1, 1) * bound, boxCol);
	DrawLine(Vector3(-1, 1, 1) * bound, Vector3(1, 1, 1) * bound, boxCol);

	DrawLine(Vector3(1, -1, 1) * bound, Vector3(1, -1, -1) * bound, boxCol);
	DrawLine(Vector3(1, -1, -1) * bound, Vector3(-1, -1, -1) * bound, boxCol);
	DrawLine(Vector3(-1, -1, -1) * bound, Vector3(-1, -1, 1) * bound, boxCol);
	DrawLine(Vector3(-1, -1, 1) * bound, Vector3(1, -1, 1) * bound, boxCol);

	DrawLine(Vector3(1, -1, 1) * bound, Vector3(1, 1, 1) * bound, boxCol);
	DrawLine(Vector3(1, -1, -1) * bound, Vector3(1, 1, -1) * bound, boxCol);
	DrawLine(Vector3(-1, -1, -1) * bound, Vector3(-1, 1, -1) * bound, boxCol);
	DrawLine(Vector3(-1, -1, 1) * bound, Vector3(-1, 1, 1) * bound, boxCol);
}

void NCL::FlockingRenderer::DrawBox(Vector3 center, float dimension) {
	DrawLine(center + Vector3(1, 1, 1) * dimension, center + Vector3(1, 1, -1) * dimension, boxCol);
	DrawLine(center + Vector3(1, 1, -1) * dimension, center + Vector3(-1, 1, -1) * dimension, boxCol);
	DrawLine(center + Vector3(-1, 1, -1) * dimension, center + Vector3(-1, 1, 1) * dimension, boxCol);
	DrawLine(center + Vector3(-1, 1, 1) * dimension, center + Vector3(1, 1, 1) * dimension, boxCol);

	DrawLine(center + Vector3(1, -1, 1) * dimension, center + Vector3(1, -1, -1) * dimension, boxCol);
	DrawLine(center + Vector3(1, -1, -1) * dimension, center + Vector3(-1, -1, -1) * dimension, boxCol);
	DrawLine(center + Vector3(-1, -1, -1) * dimension, center + Vector3(-1, -1, 1) * dimension, boxCol);
	DrawLine(center + Vector3(-1, -1, 1) * dimension, center + Vector3(1, -1, 1) * dimension, boxCol);

	DrawLine(center + Vector3(1, -1, 1) * dimension, center + Vector3(1, 1, 1) * dimension, boxCol);
	DrawLine(center + Vector3(1, -1, -1) * dimension, center + Vector3(1, 1, -1) * dimension, boxCol);
	DrawLine(center + Vector3(-1, -1, -1) * dimension, center + Vector3(-1, 1, -1) * dimension, boxCol);
	DrawLine(center + Vector3(-1, -1, 1) * dimension, center + Vector3(-1, 1, 1) * dimension, boxCol);
}

void FlockingRenderer::RenderFrame() {
	glEnable(GL_CULL_FACE);
	glClearColor(backgroundCol.x, backgroundCol.y, backgroundCol.z, backgroundCol.w);
	if (numAgents > 0) {
		RenderFlock();
	}
	glDisable(GL_CULL_FACE);
}

void FlockingRenderer::RenderFlock() {
	float screenAspect = (float)currentWidth / (float)currentHeight;
	Matrix4 viewMatrix = cam->BuildViewMatrix();
	Matrix4 projMatrix = cam->BuildProjectionMatrix(screenAspect);
	Matrix4 scaleMatrix = Matrix4::Scale(Vector3(modelScale, modelScale, modelScale));
	Vector3 camPos = cam->GetPosition();

	// Vertex Uniforms
	int projLocation = 0;
	int viewLocation = 0;
	int scaleLocation = 0;
	int boundLocation = 0;
	int gridVisualiseLocation = 0;

	// Fragment uniforms
	int camPosLocation = 0;
	int lightPosLocation = 0;
	int lightColLocation = 0;
	int useLightingLocation = 0;

	int shaderID = agentShader->GetProgramID();

	glUseProgram(shaderID);
	boundShader = agentShader;

	projLocation = glGetUniformLocation(shaderID, "projMatrix");
	viewLocation = glGetUniformLocation(shaderID, "viewMatrix");
	scaleLocation = glGetUniformLocation(shaderID, "scaleMat");
	boundLocation = glGetUniformLocation(shaderID, "bound");
	gridVisualiseLocation = glGetUniformLocation(shaderID, "visualiseGrid");

	camPosLocation = glGetUniformLocation(shaderID, "cameraPos");
	lightPosLocation = glGetUniformLocation(shaderID, "lightPos");;
	lightColLocation = glGetUniformLocation(shaderID, "lightCol");
	useLightingLocation = glGetUniformLocation(shaderID, "useLighting");

	glUniformMatrix4fv(projLocation, 1, false, (float*)&projMatrix);
	glUniformMatrix4fv(viewLocation, 1, false, (float*)&viewMatrix);
	glUniformMatrix4fv(scaleLocation, 1, false, (float*)&scaleMatrix);

	glUniform1f(boundLocation, maxBound);
	glUniform1i(useLightingLocation, useLighting);
	glUniform1i(gridVisualiseLocation, visualiseGridColours);

	glUniform3fv(camPosLocation, 1, (float*)&camPos);

	glUniform3fv(lightPosLocation, 1, (float*)&lightPosition);
	glUniform4fv(lightColLocation, 1, (float*)&lightCol);

	glBindVertexArray(agentMesh->GetVAO());
	boundMesh = agentMesh;

	glDrawElementsInstanced(GL_TRIANGLES, agentMesh->GetIndexCount(), GL_UNSIGNED_INT, (void*)0, numAgents);
}

Matrix4 FlockingRenderer::SetupDebugLineMatrix() const {
	float screenAspect = (float)currentWidth / (float)currentHeight;
	Matrix4 viewMatrix = cam->BuildViewMatrix();
	Matrix4 projMatrix = cam->BuildProjectionMatrix(screenAspect);

	return projMatrix * viewMatrix;
}

Matrix4 FlockingRenderer::SetupDebugStringMatrix() const {
	return Matrix4::Orthographic(-1, 1.0f, 100, 0, 0, 100);
}
