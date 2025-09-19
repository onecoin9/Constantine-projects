#ifndef _BUFFERMODEL_H_
#define _BUFFERMODEL_H_


#include <QObject>
#include <QHash>

typedef struct {
	
}tBufMap;

class CBufferModel : public QObject
{
	Q_OBJECT
public:


};

typedef QHash<QString, CBufferModel*> tBufferModelHash;
typedef QHash<QString, CBufferModel*>::iterator tBufferModelItr;
#endif 