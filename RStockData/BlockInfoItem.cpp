/************************************************************************/
/* 文件名称：BlockInfoItem.cpp
/* 创建时间：2013-01-08 09:28
/*
/* 描    述：用于管理板块数据
/*
/* 更新间隔：暂时定为30秒更新一次
/* 计算方式：取前十只股票进行加权平均
/************************************************************************/
#include "StdAfx.h"
#include "BlockInfoItem.h"
#include "DataEngine.h"
#include "BlockCodeManager.h"
#include "Hz2Py.h"

CBlockInfoItem::CBlockInfoItem( const QString& _file,const QString& _parent )
	: bUpdateMin(true)
	, bUpdateDay(true)
	, fIncrease(FLOAT_NAN)
	, fLTSZ(FLOAT_NAN)
	, fLastClose(FLOAT_NAN)
	, fOpenPrice(FLOAT_NAN)
	, fNewPrice(FLOAT_NAN)
	, fLowPrice(FLOAT_NAN)
	, fHighPrice(FLOAT_NAN)
{
	QFileInfo _info(_file);
	if(!_info.exists())
		return;

	parentName = _parent;
	blockFilePath = _file;
	blockName = _info.baseName();

	//更新词库表中的简拼
	shortName = CHz2Py::getHzFirstLetter(blockName);

	if(_info.isDir())
	{
		QDir dir(blockFilePath);
		QFileInfoList listEntity = dir.entryInfoList(QDir::Files|QDir::Dirs|QDir::NoDotAndDotDot);
		foreach(const QFileInfo& _f,listEntity)
		{
			if(parentName.isEmpty())
				appendBlock(new CBlockInfoItem(_f.absoluteFilePath(),blockName));
			else
				appendBlock(new CBlockInfoItem(_f.absoluteFilePath(),parentName+"|"+blockName));
		}
	}
	else
	{
		QFile file(blockFilePath);
		if(file.open(QFile::ReadOnly))
		{
			QString qsType = file.readLine().trimmed();
			if(qsType == "RegExp")
			{
				QString qsExp = file.readLine().trimmed();
				if(!qsExp.isEmpty())
				{
					QRegExp _exp(qsExp);
					_exp.setPatternSyntax(QRegExp::Wildcard);
					QList<CStockInfoItem*> listStocks = CDataEngine::getDataEngine()->getStockInfoList();
					foreach(CStockInfoItem* p,listStocks)
					{
						if(_exp.exactMatch(p->getCode()))
						{
							addStock(p);
						}
					}
				}
			}
			else
			{
				file.seek(0);
				while(!file.atEnd())
				{
					QString code = file.readLine();
					code = code.trimmed();
					if(!code.isEmpty())
					{
						addStock(CDataEngine::getDataEngine()->getStockInfoItem(code));
					}
				}
			}
			file.close();
		}
	}


	/*设置板块代码*/
	QString qsAbsPath = "";
	if(parentName.isEmpty())
		qsAbsPath = blockName;
	else
		qsAbsPath = parentName+"|"+blockName;
	blockCode = CBlockCodeManager::getBlockCode(qsAbsPath);

	connect(&timerUpdate,SIGNAL(timeout()),this,SLOT(updateData()));
	timerUpdate.start(30*1000);
	updateData();
}

CBlockInfoItem::~CBlockInfoItem(void)
{
	clearTmpData();
}

QString CBlockInfoItem::getAbsPath()
{
	if(parentName.isEmpty())
		return blockName;
	return parentName+"|"+blockName;
}

CBlockInfoItem* CBlockInfoItem::querySubBlock( const QStringList& _parent )
{
	QStringList listBlocks = _parent;
	if(listBlocks.size()<0)
		return 0;

	if(blocksInBlock.contains(listBlocks[0]))
	{
		CBlockInfoItem* pBlockItem = blocksInBlock[listBlocks[0]];
		if(listBlocks.size()==1)
			return pBlockItem;
		else
		{
			listBlocks.removeAt(0);
			return pBlockItem->querySubBlock(listBlocks);
		}
	}
	return 0;
}

bool CBlockInfoItem::isChildOf( CBlockInfoItem* parent)
{
	return getAbsPath().indexOf(parent->getAbsPath())>-1;
}

QList<CAbstractStockItem*> CBlockInfoItem::getAbsStockList()
{
	QList<CAbstractStockItem*> list;
	foreach(CBlockInfoItem* _p,blocksInBlock)
		list.push_back(_p);
	foreach(CStockInfoItem* _p,stocksInBlock)
		list.push_back(_p);

	return list;
}

QList<CStockInfoItem*> CBlockInfoItem::getStockList()
{
	return stocksInBlock;
}

QList<CBlockInfoItem*> CBlockInfoItem::getBlockList()
{
	return blocksInBlock.values();
}

bool CBlockInfoItem::hasBlocks()
{
	return blocksInBlock.size()>0 ? true : false;
}

bool CBlockInfoItem::appendStocks( QList<CStockInfoItem*> list )
{
	QStringList listCodes;
	foreach(CStockInfoItem* pItem,list)
		listCodes.push_back(pItem->getCode());

	if(appendStocks(listCodes))
	{
		foreach(CStockInfoItem* pItem,list)
		{
			if(!stocksInBlock.contains(pItem))
				stocksInBlock.push_back(pItem);
		}
		return true;
	}
	else
		return false;
}

bool CBlockInfoItem::appendStocks( QList<QString> list )
{
	if(blockFilePath.isEmpty())
		return false;

	QFile file(blockFilePath);
	if(!file.open(QFile::Append|QFile::WriteOnly))
		return false;
	foreach(const QString& e,list)
	{
		file.write(QString(e+"\r\n").toAscii());
	}
	file.close();

	return true;
}

bool CBlockInfoItem::removeStocks( QList<CStockInfoItem*> list )
{
	QStringList listCodes;
	foreach(CStockInfoItem* pItem,list)
		listCodes.push_back(pItem->getCode());

	if(removeStocks(listCodes))
	{
		foreach(CStockInfoItem* pItem,list)
		{
			stocksInBlock.removeOne(pItem);
		}
		return true;
	}
	else
		return false;
}

bool CBlockInfoItem::removeStocks( QList<QString> list )
{
	if(blockFilePath.isEmpty())
		return false;

	QMap<QString,QString> mapStocks;
	{
		//读取文件中的股票代码
		QFile file(blockFilePath);
		if(!file.open(QFile::ReadOnly))
			return false;

		while(!file.atEnd())
		{
			QString code = file.readLine();
			code = code.trimmed();
			if(!code.isEmpty())
			{
				mapStocks[code] = code;
			}
		}
		file.close();
	}
	{
		//删除对应的股票代码
		foreach(const QString& e,list)
		{
			mapStocks.remove(e);
		}
	}
	{
		//重新写回去
		QFile file(blockFilePath);
		if(!file.open(QFile::Truncate|QFile::WriteOnly))
			return false;

		QMap<QString,QString>::iterator iter = mapStocks.begin();
		while(iter!=mapStocks.end())
		{
			file.write(QString(iter.value()+"\r\n").toAscii());
			++iter;
		}
		file.close();
	}

	return true;
}

bool CBlockInfoItem::appendBlock( CBlockInfoItem* pBlock )
{
	blocksInBlock[pBlock->getBlockName()] = pBlock;
	return true;
}

void CBlockInfoItem::stockFenbiChanged( const QString& _code )
{
	bUpdateMin = true;
}

void CBlockInfoItem::stockHistoryChanged( const QString& _code )
{
	bUpdateDay = true;
}

void CBlockInfoItem::updateData()
{
	/*
	更新间隔：暂时定为1分钟更新一次
	计算方式：取前十只股票进行加权平均
	*/
	if(bUpdateMin)
	{
//		double dTotalGB = 0.0;		//总股（万）
		double dLTG = 0.0;			//流通股（万）
		double dLastClose = 0.0;	//昨日收盘价
		double dOpen = 0.0;			//今日开盘价
		double dNew = 0.0;			//最新价
		double dLow = 0.0;			//最低价
		double dHigh = 0.0;			//最高价
		int iCount = stocksInBlock.size();
		if(iCount>10)
			iCount = 10;
		for(int i=0;i<iCount;++i)
		{
			CStockInfoItem* pStock = stocksInBlock[i];
			float fLTAG = pStock->getBaseInfo()->fLtAg;		//流通股
//			dTotalGB += pStock->getBaseInfo()->fZgb;
			dLTG += fLTAG;
			dLastClose += pStock->getLastClose()*fLTAG;
			dOpen += pStock->getOpenPrice()*fLTAG;
			dNew += pStock->getNewPrice()*fLTAG;
			dLow += pStock->getLowPrice()*fLTAG;
			dHigh += pStock->getHighPrice()*fLTAG;
		}

		fLastClose = dLastClose/dLTG;
		fOpenPrice = dOpen/dLTG;
		fNewPrice = dNew/dLTG;
		fLowPrice = dLow/dLTG;
		fHighPrice = dHigh/dLTG;

		fLTSZ = dNew;
		//涨幅
		if(fNewPrice>0.0 && fLastClose>0.0)
			fIncrease = (fNewPrice-fLastClose)*100.0/fLastClose;
		bUpdateMin = false;
	}
	if(bUpdateDay)
	{

		bUpdateDay = false;
	}
}

void CBlockInfoItem::addStock( CStockInfoItem* _p )
{
	if(!_p)
		return;

	if(!stocksInBlock.contains(_p))
	{
		stocksInBlock.push_back(_p);
		connect(_p,SIGNAL(stockItemHistoryChanged(const QString&)),this,SLOT(stockHistoryChanged(const QString&)));
		connect(_p,SIGNAL(stockItemFenBiChanged(const QString&)),this,SLOT(stockFenbiChanged(const QString&)));
	}
}

void CBlockInfoItem::removeStock( CStockInfoItem* _p )
{
	if(!_p)
		return;
	stocksInBlock.removeOne(_p);
	disconnect(_p,SIGNAL(stockItemHistoryChanged(const QString&)),this,SLOT(stockHistoryChanged(const QString&)));
	disconnect(_p,SIGNAL(stockItemFenBiChanged(const QString&)),this,SLOT(stockFenbiChanged(const QString&)));
}

void CBlockInfoItem::clearTmpData()
{
	foreach(CStockInfoItem* _p,stocksInBlock)
	{
		disconnect(_p,SIGNAL(stockItemHistoryChanged(const QString&)),this,SLOT(stockHistoryChanged(const QString&)));
		disconnect(_p,SIGNAL(stockItemFenBiChanged(const QString&)),this,SLOT(stockFenbiChanged(const QString&)));
	}
	stocksInBlock.clear();
}

QString CBlockInfoItem::getCode() const
{
	return blockCode;
}

WORD CBlockInfoItem::getMarket() const
{
	return BB_MARKET_EX;
}

QString CBlockInfoItem::getName() const
{
	return blockName;
}

float CBlockInfoItem::getIncrease() const
{
	//增长
	return fIncrease;
}

float CBlockInfoItem::getVolumeRatio() const
{
	//
	return 0.0;
}

float CBlockInfoItem::getTurnRatio() const
{
	return 0.0;
}

float CBlockInfoItem::getLastClose() const
{
	return fLastClose;
}

float CBlockInfoItem::getOpenPrice() const
{
	return fOpenPrice;
}

float CBlockInfoItem::getHighPrice() const
{

	return fHighPrice;
}

float CBlockInfoItem::getLowPrice() const
{
	return fLowPrice;
}

float CBlockInfoItem::getNewPrice() const
{
	return fNewPrice;
}

float CBlockInfoItem::getTotalVolume() const
{

	return 0.0;
}

float CBlockInfoItem::getTotalAmount() const
{

	return 0.0;
}

float CBlockInfoItem::getNowVolume() const
{

	return 0.0;
}

float CBlockInfoItem::getIncSpeed() const
{

	return 0.0;
}

float CBlockInfoItem::getPriceFluctuate() const
{

	return 0.0;
}

float CBlockInfoItem::getAmplitude() const
{

	return 0.0;
}

float CBlockInfoItem::getAvePrice() const
{

	return 0.0;
}

float CBlockInfoItem::getPERatio() const
{

	return 0.0;
}

float CBlockInfoItem::getLTSZ() const
{

	return 0.0;
}

float CBlockInfoItem::getZGB() const
{

	return 0.0;
}

float CBlockInfoItem::getSellVOL() const
{

	return 0.0;
}

float CBlockInfoItem::getBuyVOL() const
{

	return 0.0;
}

float CBlockInfoItem::getBIDVOL() const
{

	return 0.0;
}

float CBlockInfoItem::getASKVOL() const
{

	return 0.0;
}

float CBlockInfoItem::getCommRatio() const
{

	return 0.0;
}

float CBlockInfoItem::getCommSent() const
{

	return 0.0;
}

float CBlockInfoItem::getLast5Volume()
{

	return 0.0;
}

float CBlockInfoItem::getMgjzc()
{

	return 0.0;
}

float CBlockInfoItem::getMgsy()
{

	return 0.0;
}

bool CBlockInfoItem::isMatch( const QString& _key )
{
	//判断代码是否匹配
	if(blockCode.indexOf(_key)>-1)
		return true;
	
	//判断名称简拼是否匹配
	for (int i = 0; i < _key.size(); ++i)
	{
		if(i>=shortName.size())
			return false;
		QList<QChar> _l = shortName[i];
		bool bMatch = false;
		foreach(const QChar& _c,_l)
		{
			if(_c == _key[i])
			{
				bMatch = true;
				break;
			}
		}
		if(!bMatch)
			return false;
	}
	return true;
}
