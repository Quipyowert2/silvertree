#ifndef TERRAINHANDLER_HPP_INCLUDED
#define TERRAINHANDLER_HPP_INCLUDED

#include <string>

class EditorMainWindow;

class TerrainHandler : public QObject {
	Q_OBJECT
public:
	TerrainHandler(EditorMainWindow& w, const std::string& id, bool feature, int button)
	    : window_(w), id_(id), feature_(feature), button_(button)
	{}
public slots:
	void terrainSelected();
private:
	EditorMainWindow& window_;
	std::string id_;
	bool feature_;
	int button_;
};

#endif
