//This is a C++ prototypes file of the menu helper functions for TestUtil.
//These are non-api functions which are called by meny functions.
//This file is used as input to LDFUTIL.EXE preprocessor to produce
//MenuDelegates.CS
#define BIO_LDF_EXPORT 1   //for benefit of LDFUTIL.EXE (while defined all prototypes exported)
void GetProcGUID();
U32 CalcMemCRC(U32 Start, U32 End)
U32 UploadToFile(U32 Start, U32 Length)
U32 FileDownload(U32 StartAddress)
