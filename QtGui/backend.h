#ifndef BACKEND_H
#define BACKEND_H

#include <QObject>
#include <QTimer>
#include <QQuickImageProvider>

struct BackEndData;

class BackEnd : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int numSamples READ numSamples NOTIFY dataChanged)
    Q_PROPERTY(int numNonLeaves READ numNonLeaves NOTIFY dataChanged)
    Q_PROPERTY(int numLeaves READ numLeaves NOTIFY dataChanged)
    Q_PROPERTY(int numNodes READ numNodes NOTIFY dataChanged)
    Q_PROPERTY(int layer READ layer NOTIFY dataChanged)
    Q_PROPERTY(int numTests READ numTests NOTIFY dataChanged)
    Q_PROPERTY(float percentCurrentTest READ percentCurrentTest NOTIFY dataChanged)
public:
    explicit BackEnd(QObject *parent = nullptr);
    virtual ~BackEnd();

    Q_INVOKABLE void read(const QString& file_url);
    Q_INVOKABLE void write(const QString& file_url);
    Q_INVOKABLE void train(const QString& imgs_dir_url, int patch_size, int overlap,
        double k, double lamda, int min_patches, int n_tests);
    Q_INVOKABLE void predict(const QString& img_url);
    Q_INVOKABLE void predictWithLowRes(const QString& img_url);
    Q_INVOKABLE void savePredicted(const QString& img_url);

    int numSamples() const;
    int numNonLeaves() const;
    int numLeaves() const;
    int numNodes() const;
    int layer() const;
    int numTests() const;
    float percentCurrentTest() const;

    QQuickImageProvider* getProvider() const;

private:
    QTimer* m_timer = nullptr;
    int n_samples = 0;
    int n_nonleaves = 0;
    int n_leaves = 0;
    int n_nodes = 0;
    int curr_layer = 0;

    BackEndData* data = nullptr;
    bool running = false;
signals:
    void dataChanged();
    void completed();
    void dataRead();
    void dataWritten();
    void imagePredicted();
    void imageSaved();

public slots:
    void updateData();
};

#endif // BACKEND_H
