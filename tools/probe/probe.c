// Codegen probe: which C loop construct makes MSVC 2.2 (/O2) emit the
// original hover.exe gwb-loop shape?
//
// Target shape (hover.exe:0x439c95):
//   xor  ecx, ecx          ; i = 0
//   mov  eax, [g_p]
//   mov  ebx, [eax+0x190]  ; nCount
//   cmp  ecx, ebx          ; entry: skip when i >= nCount
//   jge  out_of_blocks
//   mov  eax, [eax+0x198]  ; ppNode base (loaded AFTER the entry check)
// loop:
//   mov  edx, [eax]
//   cmp  dword [edx+0x20], 0
//   jne  found
//   add  eax, 4            ; ppNode++
//   inc  ecx               ; i++
//   cmp  ecx, ebx
//   jl   loop
//   cmp  ecx, ebx          ; post: if i < nCount skip the failure path
//   jl   found
// out_of_blocks:
//   ...
//
// Each fN is the same algorithm written with a different construct.
// Globals keep the optimizer from folding anything away.

int g_nCount;
int g_base;
int g_sink;

#define BASE   ((int*)g_base)
#define USED(p) (g_sink = (p))

// v1: if-guard + do-while (what we currently have - emits cmp ebx,ecx / jle)
int f1()
{
	int i = 0;
	int nCount = g_nCount;
	int* ppNode;
	if (i < nCount)
	{
		ppNode = BASE;
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
	USED((int)ppNode);
	return i;
}

// v2: plain while loop
int f2()
{
	int i = 0;
	int nCount = g_nCount;
	int* ppNode = BASE;
	while (i < nCount)
	{
		if (ppNode[0] != 0)
			break;
		ppNode++;
		i++;
	}
	if (i >= nCount)
		return -1;
	return i;
}

// v3: for loop with combined increment
int f3()
{
	int nCount = g_nCount;
	int* ppNode = BASE;
	int i;
	for (i = 0; i < nCount; i++, ppNode++)
	{
		if (ppNode[0] != 0)
			break;
	}
	if (i >= nCount)
		return -1;
	return i;
}

// v4: if-guard + do-while, guard written as the >= failure branch first
int f4()
{
	int i = 0;
	int nCount = g_nCount;
	int* ppNode;
	if (i >= nCount)
		return -1;
	ppNode = BASE;
	do
	{
		if (ppNode[0] != 0)
			break;
		ppNode++;
		i++;
	} while (i < nCount);
	if (i >= nCount)
		return -1;
	return i;
}

// v5: subtraction comparison
int f5()
{
	int i = 0;
	int nCount = g_nCount;
	int* ppNode;
	if (i - nCount < 0)
	{
		ppNode = BASE;
		do
		{
			if (ppNode[0] != 0)
				break;
			ppNode++;
			i++;
		} while (i - nCount < 0);
	}
	if (i - nCount >= 0)
		return -1;
	USED((int)ppNode);
	return i;
}

// v6: do-while, counter compared against nCount via temporary bound
int f6()
{
	int i = 0;
	int nCount = g_nCount;
	int* ppNode;
	int nLeft = nCount;
	if (nLeft > 0)
	{
		ppNode = BASE;
		do
		{
			if (ppNode[0] != 0)
				break;
			ppNode++;
			nLeft--;
		} while (nLeft > 0);
	}
	i = nCount - nLeft;
	if (i >= nCount)
		return -1;
	USED((int)ppNode);
	return i;
}

// v7: if-guard + do-while with index arithmetic instead of pointer bump
int f7()
{
	int i = 0;
	int nCount = g_nCount;
	if (i < nCount)
	{
		do
		{
			if (BASE[i] != 0)
				break;
			i++;
		} while (i < nCount);
	}
	if (i >= nCount)
		return -1;
	return i;
}

// v8: while loop with entry check duplicated (while + break, no do-while)
int f8()
{
	int i = 0;
	int nCount = g_nCount;
	int* ppNode;
	while (i < nCount)
	{
		ppNode = BASE + i;
		if (ppNode[0] != 0)
			break;
		i++;
	}
	if (i >= nCount)
		return -1;
	return i;
}
