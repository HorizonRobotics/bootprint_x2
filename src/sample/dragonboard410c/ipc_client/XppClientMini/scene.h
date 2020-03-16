#ifndef SCENE_H
#define SCENE_H


#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>

#include "sgscene.h"
#include "node/sgrectanglenode.h"
#include "node/sgtextnode.h"
 
#include "../plugin/include/interfacedevicestream.hpp"

#include "pb/x2.pb.h"

//Node pool to help SGNode alloc&free from SGScene
template<typename Node>
class SGNodePool
{
public:
    void create(SGScene* scene, int num) {
        for (int i = 0; i < num; i++) {
            Node *node = new Node;
            node->setVisible(false);
            m_pool.push(node);

            scene->addItem(node);
        }
    }
    Node* alloc() {
        Node *node = nullptr;
        if (!m_pool.empty())
        {
            node = m_pool.front();
            m_pool.pop();

            node->setVisible(true);
            m_nodes.push_back(node);
        }
        return node;
    }
    void free(Node* node) {
        if (node)
        {
            node->setVisible(false);
            m_pool.push(node);
        }
    }
    //detach all allocated nodes from scene
    void clear() {
        for (auto it : m_nodes)
        {
            free(it);
        }
        m_nodes.clear();
    }
private:
    SGScene *m_scene;
    std::queue<Node*> m_pool;//nodes not used container
    std::vector<Node*> m_nodes;//nodes used container
};

class SmartScene : public SGScene, public InterfaceMsgContainer
{
public:
    SmartScene();
    ~SmartScene();

    void drawVideo(const unsigned char* yuvptr, unsigned int len);
    void drawSkeleton(const x2::Points &points);

    virtual void recvVideoBlob(const PureVideoBlob *video, const SmartDataBlob *data) override;
    virtual void recvCommnBlob(const SmartDataBlob *data) override;

    void startStream();
    bool isStreamStop();

    void setStreamPluginPath(const char *plugin);

protected:
    void drawLine(const x2::Point & point0, const x2::Point & point1, int color);
    void streamWorker();
private:
    SGYuvVideoNode m_video;

    SGNodePool<SGRectangleNode> m_rectPool;
    SGNodePool<SGTextNode> m_textPool;

    std::thread m_thdStream;
    std::mutex m_mtxStop;
    bool m_stopStream = false;
    std::condition_variable m_cvStop;

    std::string m_pluginPath;
};

#endif // SCENE_H
