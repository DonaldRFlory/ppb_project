//-------------------------------------------------------------------------------
//    MsgBuff.h:  Header file message buff utility
// 
// 
//-------------------------------------------------------------------------------
#ifndef __MSGBUFF_H 
#define __MSGBUFF_H

#ifdef __cplusplus
   extern "C" {
#endif
U16 GetMessageBuffCount(void);
//Host side prototype: U8 ReadMessageBuff(UP_PTR_U8 Buff, U16 Count)
U8 ReadMessageBuff(U16 Count);
U8 PutMessage(U8 * String);
void MsgServe(void);


#ifdef __cplusplus
   }
#endif
 
#endif /* end __MSGBUFF_H */
//-------------------------------------------------------------------------------
//                            End Of File
//-------------------------------------------------------------------------------
