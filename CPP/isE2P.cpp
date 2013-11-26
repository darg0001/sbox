//============================================================================
// Name        : ThesisAlgorithm.cpp
// Author      : Maxim Storetvedt, Oleksandr Kazymyrov
// Description : An implementation of Browning K. A. et al. algorithm.
//============================================================================
#include "isE2P.h"

void										clearColumn(mzd_t *, unsigned long long column);
mzd_t* 										findMatrix(map<unsigned long long, vector< mzd_t* > >, unsigned long long, bool);
map<unsigned long long, vector< mzd_t* > >	findSigmas(unsigned long long*, unsigned long long);
bool 										matrixCheck(mzd_t *, unsigned long long, map<unsigned long long, vector< mzd_t* > >);
void										print_matrix(mzd_t const *, string);
void										print_Sigmas(map<unsigned long long, vector< mzd_t* > >);
mzd_t* 										tryCombine(mzd_t*, vector< mzd_t* >);
void										updatedMat(unsigned long long *, unsigned long long, mzd_t *);

int test()
{
	unsigned long long Sbox[] = {0, 50, 7, 40, 19, 23, 47, 54, 41, 34, 57, 47, 35, 30, 8, 40, 53, 6, 61, 19, 19, 22, 32, 56, 62, 52, 33, 54, 1, 61, 37, 4, 4, 33, 18, 42, 37, 54, 8, 6, 59, 39, 58, 59, 3, 41, 57, 14, 30, 58, 7, 62, 10, 24, 40, 39, 3, 30, 13, 13, 14, 37, 59, 13};
	unsigned long long n = 6, length = 1<<n;

	if (is_E2P(Sbox,length,n))
	{
		printf("No functions were found\n");
	}
	else
	{
		printf("No functions were found\n");
	}

	return 0;
}

int is_E2P(unsigned long long *sbox, unsigned long long length, unsigned long long n)
{
	mzd_t *M = NULL;
	map<unsigned long long, vector< mzd_t* > > Sigmas;
	unsigned long long i = 0;

	if(sbox[0] != 0)
	{
		for(i=1;i<length;i++)
			sbox[i] = sbox[0] ^ sbox[i];
		sbox[0] = 0;
	}

	M = mzd_init(n<<1, n<<1);

	printf("length = %lld\n",length);
	printf("Find Sigmas\n");

	Sigmas = findSigmas(sbox, n);

	// Print sbox
	printf("sbox:\n");
	for(i = 0; i < length; i++)
	{
		printf("%02X ", sbox[i]);
		if((i+1)%8 == 0)
			printf("\n");
	}
	printf("\n");

	// Print sigmas
	//print_Sigmas(Sigmas);
	printf("Sigmas:\n");
	for(i = 0; i < Sigmas.size(); i++)
	{
		printf("Sigmas[%lld]: %lld\n",i,Sigmas[i].size());
	}
	printf("\n");	

	M = findMatrix(Sigmas,n,true);

	// Print the matrix
	if(!mzd_is_zero(M))
	{
		print_matrix(M,"M");
	}

	mzd_free(M);

	return 1;
}

/*
 * Clears the column of the matrix
 */
void clearColumn(mzd_t *L, unsigned long long column)
{
	unsigned long long i = 0;

	for(i=0; i < L->nrows; i++)
		mzd_write_bit(L,i,column,0);
}

mzd_t* findMatrix(map<unsigned long long, vector< mzd_t* > > Sigmas, unsigned long long n, bool full = false)
{
	// Variables for testing performance
	clock_t time_updatedMat[3] = {0,0,0}, time_gauss[3] = {0,0,0}, time_matrixCheck[3] = {0,0,0}, time_find[3] = {0,0,0}, time_tryCombine[3] = {0,0,0};

	//  The working matrices
	mzd_t *L, *M, *T;

	// The found matrices will be stored here
	vector< mzd_t* > foundM, foundL;

	// Additional variables
	unsigned long long i = 0, rank = 0, column = 0;

	// The progress tracker and the tracker of max values 
	unsigned long long *progressTracker = NULL, *maxValueTracker = NULL;

	// Initialises the empty matrices
	T = mzd_init(n, n<<1);
	L = mzd_init(n, n<<1);
	M = mzd_init(n<<1, n<<1);

	// We will generate our columns here, in decimal form
	progressTracker = (unsigned long long*)calloc(n<<1,sizeof(unsigned long long));

	// Defines the highest achieveable value for a column. This is 2^(n-1) for most
	// except the identity matrix part.
	maxValueTracker = (unsigned long long*)calloc(n<<1,sizeof(unsigned long long));

	for(i = 0; i < n<<1; i++)
	{
		if(i < n)
		{
			maxValueTracker[i] = (1 << (i+1)) - 1;
		}
		else
		{
			maxValueTracker[i] = (1 << n) - 1;
		}
	}

	// Several prints for debugging
	// print_matrix(L,"L");

	// for(i = 0; i < n<<1; i++)
	// 	printf("progressTracker[%lld] = %lld\n",i,progressTracker[i]);

	// for(i = 0; i < n<<1; i++)
	// 	printf("maxValueTracker[%lld] = %lld\n",i,maxValueTracker[i]);

	while(true)
	{
		// if(progressTracker[6] == 0)
		// {
		// 	// printf("progressTracker = [");
		// 	// for(i = 0; i < 2*n; i++)
		// 	// {
		// 	// 	if(i != (2*n - 1) )
		// 	// 		printf("%d,", progressTracker[i]);
		// 	// 	else
		// 	// 		printf("%d] (%ld) // (%lld,%d)\n", progressTracker[i],foundMatrices->size(),column,maxValueTracker[column]);
		// 	// }

		// 	printf("time_updatedMat\t\t: %f\n", (double)(time_updatedMat[2]) / CLOCKS_PER_SEC);
		// 	printf("time_gauss\t\t: %f\n", (double)(time_gauss[2]) / CLOCKS_PER_SEC);
		// 	printf("time_matrixCheck\t: %f\n", (double)(time_matrixCheck[2]) / CLOCKS_PER_SEC);
		// 	printf("time_find\t\t: %f\n", (double)(time_find[2]) / CLOCKS_PER_SEC);
		// 	printf("time_tryCombine\t\t: %f\n", (double)(time_tryCombine[2]) / CLOCKS_PER_SEC);
		// 	printf("~~~~~~~~~~~~~~~~~~~~~\n");
		// }

		if (progressTracker[column] > maxValueTracker[column])
		{
			progressTracker[column] = 0;
			clearColumn(L, column);
			column--;
			if(column == (unsigned long long)(-1))
			{
				break;
			}
			progressTracker[column]++;
			continue;
		}
	
		time_updatedMat[0] = clock();
		updatedMat(progressTracker, column, L);
		time_updatedMat[1] = clock();
		time_updatedMat[2] += (time_updatedMat[1] - time_updatedMat[0]);

		// print_matrix(L,"L");

		time_gauss[0] = clock();
		mzd_copy(T,L);
		rank = mzd_echelonize_pluq(L,1);
		time_gauss[1] = clock();
		time_gauss[2] += (time_gauss[1] - time_gauss[0]);

		// print_matrix(L,"L");
		// print_matrix(T,"T");
		// printf("~~~~~~~~~~~~~~~~~~~~~\n");

		// printf("(rank,column) = (%lld,%lld)\n", rank, column);

		if( (!mzd_equal(T,L)) or ( ( column == (2*n-1) )  and (rank != n)  ) ) 
		{
			progressTracker[column]++;
			continue;
		}

		time_matrixCheck[0] = clock();
		if(matrixCheck(L, column, Sigmas))
		{
			time_matrixCheck[1] = clock();
			time_matrixCheck[2] += (time_matrixCheck[1] - time_matrixCheck[0]);
			if( column == (2*n-1) )
			{
				time_find[0] = clock();
				// Is the following if redundant? It make sence when foundL is predefined.
				// It is nessessary to check this.
				if(!(find(foundL.begin(), foundL.end(), L) != foundL.end()))
				{
					time_find[1] = clock();
					time_find[2] += (time_find[1] - time_find[0]);
					printf("progressTracker = [");
					for(i = 0; i < 2*n; i++)
					{
						if(i != (2*n - 1) )
							printf("%d,", progressTracker[i]);
						else
							printf("%d] (%ld)\n", progressTracker[i],foundL.size() + 1);
					}
					// printf(">> %d\n",__LINE__);
					time_tryCombine[0] = clock();
					M = tryCombine(L,foundL);
					time_tryCombine[1] = clock();
					time_tryCombine[2] += (time_tryCombine[1] - time_tryCombine[0]);

					if (mzd_is_zero(M))
					{
						// printf(">> %d\n",__LINE__);
						foundL.push_back(mzd_copy(NULL,L));
					}
					else
					{
						// printf(">> %d\n",__LINE__);
						if(full == true)
						{
							foundM.push_back(mzd_copy(NULL,M));
						}
						else
						{
							if(progressTracker)
								free(progressTracker);
							if(maxValueTracker)
								free(maxValueTracker);

							mzd_free(T);
							mzd_free(L);

							return M;
						}
					}
				}
				else
				{
					printf(">>> Critical bug (%d) (see comments under \"if\")<<<\n",__LINE__);
					exit(0);
					time_find[1] = clock();
					time_find[2] += (time_find[1] - time_find[0]);
				}
				progressTracker[column]++;
			}
			else
			{
				// printf(">> %d\n",__LINE__);
				column++;
			}
		}
		else
		{
			time_matrixCheck[1] = clock();
			time_matrixCheck[2] += (time_matrixCheck[1] - time_matrixCheck[0]);
			progressTracker[column]++;
		}
	}

	printf("Done. Number of linear functions is %ld\n", foundL.size());
	printf("Number of matriceis is %ld\n", foundM.size());

	if(progressTracker)
		free(progressTracker);
	if(maxValueTracker)
		free(maxValueTracker);

	mzd_free(T);
	mzd_free(L);
	mzd_free(M);

 	return mzd_init(n<<1, n<<1);;
}

/*
 * Returns the map of Sigmas
 */
map<unsigned long long, vector< mzd_t* > > findSigmas(unsigned long long *F, unsigned long long n)
{
	map<unsigned long long, vector< mzd_t* > > Sigmas;

	unsigned long long xy = 0, Fxy = 0, i = 0, j = 0, k = 0, nbits = 0;
	mzd_t* sigma;

	sigma = mzd_init(n<<1, 1);

	for (i = 0; i < (unsigned long long)(1<<n); i++)
	{
		for (j = i+1; j < (unsigned long long)(1<<n); j++)
		{
			nbits = 0;
			xy = i^j;
			Fxy = (F[i]^F[j]);

			for(k = 0; k < n; k++)
			{
				mzd_write_bit(sigma,k,0,(xy >> k) & 1);
				mzd_write_bit(sigma,n+k,0,(Fxy >> k) & 1);
			}

			for(k = 2*n - 1; k < 2*n; k--)
			{
				if (mzd_read_bit(sigma,k,0) == 1)
				{
					nbits = k;
					break;
				}
			}

			// if (nbits == 2)
			// {
			// 	printf("================\n");
			// 	printf(" xy = %02llX\n", xy);
			// 	printf("Fxy = %02llX\n", Fxy);
			// 	mzd_print(sigma);
			// 	printf("nbits = %lld\n", nbits);
			// 	printf("================\n");
			// }

			// Add only unique vectors
			if (find(Sigmas[nbits].begin(), Sigmas[nbits].end(), sigma) == Sigmas[nbits].end())
			{
				Sigmas[nbits].push_back(mzd_copy(NULL,sigma));
			}
		}
	}

	mzd_free(sigma);

	return Sigmas;
}

/**
 * Iterates through the columns of a matrix, and checks if they all are accepted with their corresponding sigmas
 */
bool matrixCheck(mzd_t *L, unsigned long long column, map<unsigned long long, vector< mzd_t* > > Sigmas)
{
	unsigned long long s = 0;
	mzd_t *T = NULL;

	T = mzd_init(L->nrows,1);

	for(s=0;s<Sigmas[column].size();s++)
	{
		// printf("T:\t\t");
		// mzd_info(T,0);
		// printf("L:\t\t");
		// mzd_info(L,0);
		// printf("Sigma[%lld][%lld]:\t",column,s);
		// mzd_info(Sigmas[column][s],0);
		mzd_mul(T, L, Sigmas[column][s], 0);

		if (mzd_is_zero(T) != 0)
		{
			mzd_free(T);
			return false;
		}
	}

	mzd_free(T);

	return true;
}

// Print the given matrix.
void print_matrix(mzd_t const *L, string str)
{
	unsigned long long i = 0, j = 0;

	cout << str << ":" << endl;

	for (i = 0; i < L->nrows; i++)
	{
		for(j = 0; j < L->ncols; j++)
		{
			cout << mzd_read_bit(L,i,j) << " ";
		}
		cout << endl;
	}
}

// Print Sigmas
void print_Sigmas(map<unsigned long long, vector< mzd_t* > > Sigmas)
{
	unsigned long long i = 0, j = 0, s = 0, m = 0;

	cout << "Sigmas:" << endl;

	for (s = 0; s < Sigmas.size(); s++)
	{
		cout << "Sigmas[" << s << "] (" << Sigmas[s].size() << ")" << " = [";
		for (m = 0; m < Sigmas[s].size(); m++)
		{
			cout << "[";
			for (i = 0; i < Sigmas[s][m]->nrows; i++)
			{
				for(j = 0; j < Sigmas[s][m]->ncols; j++)
				{
					cout << mzd_read_bit(Sigmas[s][m],i,j) << " ";
				}
			}
			if (m != Sigmas[s].size()-1)
				cout << "],";
			else
				cout << "]";
		}
		cout << "]" << endl;
	}
}

/*
 * Updates the column of the matrix
 */
void updatedMat(unsigned long long *progressTracker, unsigned long long column, mzd_t *L)
{
	unsigned long long i = 0;

	for(i=0; i < L->nrows; i++)
		mzd_write_bit(L,i,column,(progressTracker[column] >> i) & 1);
}

/*
 * Try to combines two n x 2n matrices into one invertible 2n x 2n matrix.
 */
mzd_t* tryCombine(mzd_t *L, vector< mzd_t* > foundL)
{
	unsigned long long rank = 0, k = 0, n = L->nrows;
	mzd_t *M = NULL, *T = NULL;

	M = mzd_init(n<<1,n<<1);
	T = mzd_init(n<<1,n<<1);

	for(k = 0; k < foundL.size(); k++)
	{
		mzd_stack(M,L,foundL[k]);
		mzd_copy(T,M);

		rank = mzd_echelonize_pluq(T,1);

		if( rank == (n<<1) )
		{
			mzd_free(T);
			return M;
		}
	}

	mzd_free(T);
	mzd_free(M);

	return mzd_init(n<<1,n<<1);
}
