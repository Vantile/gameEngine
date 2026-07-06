#include <VulkanRenderer.h>

int main(int argc, char* argv[])
{
	VulkanRenderer renderer;

	renderer.Init();
	
	renderer.Run();

	renderer.Cleanup();

	return 0;
}
