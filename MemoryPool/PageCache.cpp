#include "PageCaceh.h"

PageCache PageCache::_inst;

//Ϊʲô��ֱ��ȥϵͳ������2ҳ������������
//Ƶ�������С�Ŀռ䣬�����Ч�ʵĽ���

void PageCache::CreatePageidToSpanMap(Span* span)
{
	for (int i = 0; i < span->_npage; ++i)
	{
		_id_span_map[span->_pageid + i] = span;
	}
}

Span* PageCache::NewSpan(size_t npage)
{
	if (!_pagelist[npage].Empty())
	{
		return _pagelist[npage].PopFront();
	}

	for (size_t i = npage+1; i < NPAGES; ++i)
	{
		SpanList* pagelist = &_pagelist[i];
		//��pagelist��Ӧ��Ͱ���м���
		std::unique_lock<std::mutex> lock(pagelist->_mtx);
		
		if (!pagelist->Empty())                          //ѭ�����������page ,����в�Ϊ�յľ�ȡ���е�һ��span ��
		{
			Span* span = pagelist->PopFront();
			Span* split = new Span;
			split->_pageid = span->_pageid + span->_npage - npage;
			split->_npage = npage;
			span->_npage -= npage;

			_pagelist[span->_npage].PushFront(span);

			//��������Ŀ���span��ҳ��֮��Ĺ�ϵ
			CreatePageidToSpanMap(split);

			return split;
		}
	}

	// ��Ҫ��ϵͳ�����ڴ�  �ڴ������128 ���ǹ̶��ģ����Զ����
	void* ptr = VirtualAlloc(NULL, (NPAGES-1)<<PAGE_SHIFT, MEM_RESERVE| MEM_COMMIT , PAGE_READWRITE);  //���ҳ����129 ��ԭ������Ϊ������0 
	if (ptr == nullptr)
	{
		throw std::bad_alloc();
	}

	Span* largespan = new Span;
	largespan->_pageid = (PageID)ptr >> PAGE_SHIFT;
	largespan->_npage = NPAGES - 1;

	_pagelist[NPAGES - 1].PushFront(largespan);
	
     //�����ڴ�����ҳ��Ҳ��span����ӳ��
	CreatePageidToSpanMap(largespan);

    
	return NewSpan(npage);
}


Span* PageCache::MapObjectToSpan(void* obj)
{
	//���ҳ��
	PageID pageid = (PageID)obj >> PAGE_SHIFT;
	//����pageid �ҵ���Ӧ��span�ĵ�����
	auto it = _id_span_map.find(pageid);
	return it->second;
}


//˫��Ĵ�ͷ��ѭ�����������ɾ���ȵ��߼�
//��span ���кϲ�
void PageCache::ReleaseSpanToPageCahce(Span* span){
	
	//�ҵ���ǰ��Ҫ�����ҳ��ǰһҳ��span
	auto previt = _id_span_map.find(span->_pageid - 1);
	SpanList* pagelist = &PageCache::GetInstance()->_pagelist[previt->second->_npage];
	std::unique_lock<std::mutex> lock_prev(pagelist->_mtx);
	while (previt->second != pagelist->begin()){
		if (previt->second->_usecount != 0)
			break;
		if (span->_npage >= 128)//�ϲ�ʱҪ���п��ƣ����ܻ�ϲ���һ���ر���ҳ�����ܳ���128
			break;

		pagelist->Erase(previt->second);
		span->_pageid = previt->second->_pageid;
		span->_npage += previt->second->_npage;
		delete previt->second;
		
		previt = _id_span_map.find(span->_pageid - 1);
		SpanList* pagelist = &PageCache::GetInstance()->_pagelist[previt->second->_npage];
	}

	auto nextit = _id_span_map.find(span->_pageid + span->_npage);
	pagelist = &PageCache::GetInstance()->_pagelist[nextit->second->_npage];
	std::unique_lock<std::mutex> lock_next(pagelist->_mtx);
	while (nextit->second != pagelist->end()){

		if (nextit->second->_usecount != 0)
			break;
		if (span->_npage >= 128)
			break;

		pagelist->Erase(nextit->second);
		span->_npage += nextit->second->_npage;
		delete nextit->second;

		nextit = _id_span_map.find(span->_pageid + span->_npage);
		pagelist = &PageCache::GetInstance()->_pagelist[nextit->second->_npage];
	}
	//�ϲ������map�ĸ���
	CreatePageidToSpanMap(span);
	//����
	PageCache::GetInstance()->_pagelist[span->_npage].PushFront(span);
	
}
