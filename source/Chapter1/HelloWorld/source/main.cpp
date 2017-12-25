#include <vulkan/vulkan.h>

#include <QMainWindow>
#include <QGridLayout>
#include <QApplication>

#include "HelloVulkanApp.h"

VulkanApp* g_pApp = NULL;

// Allow the user to specify the Vulkan instance extensions
std::vector<const char *> instanceExtensionNames = {
	VK_KHR_SURFACE_EXTENSION_NAME,
	VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
	VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
};

class  MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow() : QMainWindow()
	{
		QWidget* centralWidget = new QWidget(this);
		QHBoxLayout* layout = new QHBoxLayout();
		setMinimumSize(640, 480);
		centralWidget->setLayout(layout);

		// Create a new instance of HelloVulkanApp object
		g_pApp = new HelloVulkanApp();

		// Initialize the application
		g_pApp->Init();

		layout->addWidget(QWidget::createWindowContainer(g_pApp->GetApplicationWindow()));
		setCentralWidget(centralWidget);
	};

	~MainWindow() 
	{
		// Call application to cleanup
		g_pApp->Cleanup();
	};
};

int main(int argc, char **argv)
{
	QApplication app(argc, argv);

	MainWindow* window = new MainWindow();
	window->setWindowTitle("Vulkan Hello World in Qt window");
	window->resize(640, 480);
	window->show();
	window->adjustSize();

	app.exec();
	return 0;
}

#include "main.moc"