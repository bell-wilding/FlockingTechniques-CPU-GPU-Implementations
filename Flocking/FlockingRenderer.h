#pragma once
#include "../Plugins/OpenGLRendering/OGLRenderer.h"
#include "../Plugins/OpenGLRendering/OGLShader.h"
#include "../Plugins/OpenGLRendering/OGLTexture.h"
#include "../Plugins/OpenGLRendering/OGLMesh.h"
#include "../Common/Matrix4.h"
#include "../Common/Camera.h"

namespace NCL {
	class Maths::Vector3;
	class Maths::Vector4;
	class Flock;

	class FlockingRenderer : public OGLRenderer {
	public:
		FlockingRenderer();
		~FlockingRenderer();

		OGLShader* GetAgentShader() { return agentShader; }
		OGLMesh* GetAgentMesh() { return agentMesh; }

		void InitFlock(GLuint bufFlock, int numAgents, float maxBound, float modelScale = 50);

		void ToggleLighting();

		void ToggleDarkMode();
		Vector4 GetBoxColour() { return boxCol; }

		void ToggleGridVisualisation();

		void UpdateCamera(float dt) { cam->UpdateCamera(dt); }

		void DrawBoundingBox(float bound);

		void DrawBox(Vector3 center, float dimension);

		Camera* GetCamera() { return cam; }

		Vector2 GetWindowDimensions() const { return Vector2(currentWidth, currentHeight); }

	protected:
		void RenderFrame()	override;
		void RenderFlock();
		void InitCamera();

		Matrix4 SetupDebugLineMatrix()		const override;
		Matrix4 SetupDebugStringMatrix()	const override;

		OGLShader* agentShader;
		OGLMesh* agentMesh;

		GLuint bufFlock;
		int numAgents;
		float maxBound;
		float modelScale;

		Camera* cam;

		bool darkMode;
		Vector4 backgroundCol;
		Vector4 boxCol;

		Vector4 lightCol;
		float lightRadius;
		Vector3 lightPosition;
		bool useLighting;

		bool visualiseGridColours;
	};
}