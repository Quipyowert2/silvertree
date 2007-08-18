#ifndef EDITORMAINWINDOW_HPP_INCLUDED
#define EDITORMAINWINDOW_HPP_INCLUDED

#include <QtGui/QToolButton>
#include <vector>

#include "../wml_node_fwd.hpp"
#include "ui_mainwindow.hpp"

class EditorMainWindow : public QMainWindow
{
	Q_OBJECT

	public:
		EditorMainWindow(QWidget *parent = 0); 
		bool openScenario(const char* file);
		bool openMap(const char *file);
	
	public slots:
		void openRequested();
		void saveRequested();
		void zoominRequested();
		void zoomoutRequested();
		void horzScroll(int value);
		void vertScroll(int value);
		void rotateLeft();
		void rotateRight();
		void tiltUp();
		void tiltDown();
		void undo();
		void redo();
		void setTerrain(const std::string& id, bool feature, int button);

	private:
		Ui::MainWindow ui;
		hex::camera *camera_;
		hex::gamemap *map_;
		wml::node_ptr scenario_;
		bool opened_;
		std::vector<class TerrainHandler*> handlers_;
		std::vector<QToolButton*> tool_buttons_;
};

#endif
