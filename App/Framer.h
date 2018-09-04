/*=============== Framer. h ===============*/
/*
BY:	Aravind Dhulipalla
		16.472 / 16.572 Embedded Real Time Systems
		Electrical and Computer Engineering Dept.
		UMASS Lowell
		
Purpose
Interface to Framer.c.
*/

#ifndef Framer_H
#define Framer_H

/*---------Function Prototypes--------------*/
CPU_VOID CreateFramerTask();
void Framedata(CPU_INT08U Robot,CPU_INT08U Pkt_type,CPU_INT08U Msg_code);
CPU_VOID FramerTask(CPU_VOID *data);

#endif