//*****************************************************************************
//
//	SegList.h
//  Author   DF
//  COMPILER    -      Keil uVision Version 5.15
//  TARGET      -      Cortex M3
//
//           
//*****************************************************************************
#define SEG_LIST_LEN 8
#define MIN_SEG_TICKS 5

typedef struct 
{
	LLUnion LLParam1;
	LLUnion LLParam2;
	unsigned long EndTick;
	unsigned char Flags;
} SEG_DATA;

//If we have SEG_FLAG_CV, the LLParam1 is velocity and LLParam2 is only used if
//we want to jam Position or Velocity at end of move and which is determined
//by SEG_FLAG_SEL_VEL being set or not.
//withoug SEG_FLAG_CV, LLParam1 is acceleration, and LParam2 is End-Position or
//End-Velocity for jamming use.
//Since we cannot have JAM_VEL and JAM_POS, we 
//have SEG_FLAG_VEL to select velocity for jamming at end of segment
//otherwise we would jam position. Jamming occurs only if SEG_FLAG_JAM is set
//so SEG_FLAG_VEL by itself has no effect (at present time)
#define SEG_FLAG_VEL  1
#define SEG_FLAG_JAM  2
//This is a constant velocity segment.
#define SEG_FLAG_CV	  4
#define SEG_FLAG_FINAL 8

void SegmentProcess0(void);
void SegmentProcess1(void);
void SegmentProcess2(void);
void SegmentProcess3(void);
void AccVelProcess0(void);
void AccVelProcess1(void);
void AccVelProcess2(void);
void AccVelProcess3(void);
U8 StartPList(float Accel, float ScaleA, float ScaleB);
void PListServe(void);
void InitPointList(void);

U8 ClearSegList(U8 Channel);
U32 SLFreeSpace(U8 Channel);
U8 SegCompute(U8 Channel, double DeltaPos, double VEnd, unsigned long DeltaTicks, long long EndPos, U8 Flags);
U8 DoMoveTest(void);
void DoTest(unsigned char Channel, unsigned char VMode, unsigned char AMode, long Steps);

//*****************************************************************************
// End file SegList.h
//*****************************************************************************/
