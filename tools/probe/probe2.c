// Probe 2: bound kept live in a callee-saved register across the loop.
struct S { char pad[0x190]; int nCount; int base; };
struct S* g_p;
int g_sink;

int g1()   // if+do-while, bound reused after the loop
{
	int i = 0;
	int* ppNode;
	int nCount = g_p->nCount;
	if (i < nCount)
	{
		ppNode = (int*)g_p->base;
		do
		{
			if (ppNode[0] != 0)
				break;
			ppNode++;
			i++;
		} while (i < nCount);
	}
	if (i >= nCount)
		return -1;
	g_sink = (int)ppNode;
	return nCount - i;   // keep nCount live past the loop
}

int g2()   // for-loop, bound reused after the loop
{
	int* ppNode;
	int nCount = g_p->nCount;
	int i;
	ppNode = (int*)g_p->base;
	for (i = 0; i < nCount; ppNode++, i++)
	{
		if (ppNode[0] != 0)
			break;
	}
	if (i >= nCount)
		return -1;
	g_sink = (int)ppNode;
	return nCount - i;
}

int g3()   // while+&& idiom used elsewhere in hover.cpp
{
	int i = 0;
	int nCount = g_p->nCount;
	int* ppNode = (int*)g_p->base;
	while (i < nCount && ppNode[0] == 0)
	{
		ppNode++;
		i++;
	}
	if (i >= nCount)
		return -1;
	g_sink = (int)ppNode;
	return nCount - i;
}
