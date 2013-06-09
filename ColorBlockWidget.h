/************************************************************************/
/* �ļ����ƣ�ColorBlockWidget.h
/* ����ʱ�䣺2012-11-19 10:59
/*
/* ��    ����������ʾ��ֻ��Ʊ�����ƣ��̳���CBaseWidget
/*           �����Ե�ʹ�ò�ͬ��ɫ������ʾ��Ʊ���������ƣ��Ӷ�����ͬ�������У�
/*           չ�ָ������Ϣ��
/************************************************************************/

#ifndef COLOR_BLOCK_WIDGET_H
#define COLOR_BLOCK_WIDGET_H
#include "BaseBlockWidget.h"
#include "BlockInfoItem.h"

class CColorBlockWidget : public CBaseBlockWidget
{
	Q_OBJECT
public:
	CColorBlockWidget(CBaseWidget* parent = 0);
	~CColorBlockWidget(void);

public:
	//���ظ�K��ͼ��������Ϣ
	virtual bool loadPanelInfo(const QDomElement& eleWidget);
	//�����K��ͼ��������Ϣ
	virtual bool savePanelInfo(QDomDocument& doc,QDomElement& eleWidget);

	//��������
	virtual void updateData();
	//������ǰ���ڴ�
	virtual void clearTmpData();

	//���µ�ǰ������ʽ
	virtual void updateSortMode(bool bSelFirst);
public:
	//ͨ������keyword��ȡ��Ҫ�ڰ�����������ʾ������
	virtual void getKeyWizData(const QString& keyword,QList<KeyWizData*>& listRet);
	//���̾��鴰��ȷ�Ϻ󴥷�
	virtual void keyWizEntered(KeyWizData* pData);

public slots:
	virtual void setBlock(const QString& block);
	virtual void setCircle(RStockCircle _c);		//���õ�ǰ����ʾ����

protected slots:
	void onSetCurrentBlock();								//���õ�ǰ��ʾ�İ��
	void onSetExpression();									//���õ�ǰ��ʾ�ı���ʽ
	void updateColorBlockData();							//���µ�ǰ��Ҫ��ʾ������
	void updateShowMap();									//����Ҫ��ʾ�����ݣ���ɾ��֮ǰ��

private:
	void clickedStock(CStockInfoItem* pItem);	//�������Ʊʱ����

protected:
	virtual void paintEvent(QPaintEvent* e);				//�����¼�
	virtual void mouseMoveEvent(QMouseEvent* e);			//����ƶ��¼�
	virtual void mousePressEvent(QMouseEvent* e);			//������¼�
	virtual void wheelEvent(QWheelEvent* e);				//����м������¼�
	virtual void keyPressEvent(QKeyEvent* e);				//���̲���

	//����ɫ��
	//vColor:ȡ0-21
	//vHeight:�ٷֱ�0-100%;
	//vWidth:�ٷֱ�0%-100%;
	void drawColocBlock(QPainter& p,int iY,
		QVector<float>& vColor,QVector<float>& vHeight,QVector<float>& vWidth);

	//�麯���������ؼ����Զ���˵���
	virtual QMenu* getCustomMenu();

	/*��ػ�ͼ����*/
private:
	void drawHeader(QPainter& p,const QRect& rtHeader);			//����ͷ����Ϣ
	void drawClient(QPainter& p,const QRect& rtClient);			//����������
	void drawBottom(QPainter& p,const QRect& rtBottom);			//���Ƶײ�����

	void drawStock(QPainter& p,const QRect& rtCB,CStockInfoItem* pItem);	//���Ƶ�ֻ��Ʊ

	QRect rectOfStock(CStockInfoItem* pItem);					//��ȡĳֻ��Ʊ��ʾ��λ��
	CStockInfoItem* hitTestStock(const QPoint& ptPoint) const;		//����ĳ����ָ��Ĺ�Ʊ��Ϣ
	RStockData* hitTestCBItem(const QPoint& ptPoint) const;//����ĳ����ָ���ɫ����Ϣ

private:
	QMenu* m_pMenuBlockList;				//���а����Ϣ����ǰѡ�еİ���ϴ򹴣�

	CBlockInfoItem* m_pBlock;				//��ǰ�İ������
	QList<CStockInfoItem*> m_listStocks;	//��ǰ��ʾ�����й�Ʊ�б�

	QMap<CStockInfoItem*,int> m_mapStockIndex;	//�������ٲ���ĳֻ��Ʊ���ڵ�����
	QMap<CStockInfoItem*,QMap<time_t,RStockData*>*> mapStockColorBlocks;	//��ǰ��ʾ��ColorBlock����
	CStockInfoItem* m_pSelectedStock;			//��ǰѡ�еĹ�Ʊ

	/*���ڻ��Ʋ����ĳ�Ա����*/
private:
	int m_iTitleHeight;						//ͷ���߶�
	int m_iBottomHeight;					//�ײ��ĸ߶�
	int showStockIndex;						//��ǰ��ʾ����ʼλ�ã��У�

	QRect m_rtHeader;						//ͷ��Header����
	QRect m_rtBottom;						//�ײ�������������������Ϣ

	QTimer m_timerUpdateUI;					//������µ�timer

	QString m_qsExpColor;					//��ɫ����ʽ
	QString m_qsExpHeight;					//�߶ȱ���ʽ
	QString m_qsExpWidth;					//���ȱ���ʽ

	//�����������ڵľ���
	QMap<int,QRect> m_mapCircles;
};

#endif	//COLOR_BLOCK_WIDGET_H