#pragma once

#include "Common.h"
#include <map>
class PageCache
{
public:
	static PageCache* GetInstance()
	{
		return &_inst;
	}
	//��ȡһ���µ�span
	Span* NewSpan(size_t npage);

	//����ӳ��
	void CreatePageidToSpanMap(Span* span);
	// ��ȡ�Ӷ���span��ӳ��
	Span* MapObjectToSpan(void* obj);
	// �ͷſ���span�ص�Pagecache�����ϲ����ڵ�span
	void ReleaseSpanToPageCahce(Span* span);


private:
	SpanList _pagelist[NPAGES];
private:
	PageCache() = default;
	PageCache(const PageCache&) = delete;
	static PageCache _inst;

	//����һ��span ��ҳ��֮���ӳ��map
	std::map<PageID, Span*> _id_span_map;
};