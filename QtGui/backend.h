#ifndef BACKEND_H
#define BACKEND_H

#include <QObject>
#include <QTimer>

struct BackEndData;

class BackEnd : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int numSamples READ NumSamples NOTIFY DataChanged)
    Q_PROPERTY(int numNonLeaves READ NumNonLeaves NOTIFY DataChanged)
    Q_PROPERTY(int numLeaves READ NumLeaves NOTIFY DataChanged)
    Q_PROPERTY(int numNodes READ NumNodes NOTIFY DataChanged)
    Q_PROPERTY(int layer READ Layer NOTIFY DataChanged)
public:
    explicit BackEnd(QObject *parent = nullptr);
    virtual ~BackEnd();

    Q_INVOKABLE void Read(const QString& path);
    Q_INVOKABLE void Write(const QString& path);
    Q_INVOKABLE void Train(const QString& imgsDirPath);


    int NumSamples() const;
    int NumNonLeaves() const;
    int NumLeaves() const;
    int NumNodes() const;
    int Layer() const;

private:
    QTimer* m_timer = nullptr;
    int n_samples = 0;
    int n_nonleaves = 0;
    int n_leaves = 0;
    int n_nodes = 0;
    int layer = 0;

    BackEndData* data = nullptr;
    bool running = false;
signals:
    void DataChanged();

public slots:
    void UpdateData();
};

#endif // BACKEND_H
