#ifndef __SP_ERRDEF_H__
#define __SP_ERRDEF_H__


#define COMMOM_ERROR "��ͨ����"
#define TYPE_ERROR "�������"
#define LEN_ERROR "���ȴ���"

/*
*  E--����
*  R--����
*  S--��̨����
*/
#define SP_SUCCESS 0x00
#define SP_FAIL 0x01
#define SP_E_FLASH_NOT_FOUNT 0x02 //flashδ�ҵ�
#define SP_E_FLASH_READ 0x03 //flash��ȡʧ��
#define SP_E_FLASH_CRC 0x04 //flashУ��ʧ��
#define SP_E_FLASH_WRITE 0x05 //flashдʧ��
#define SP_E_FLASH_SEQNO_NOT_EQUAL 0x06 //������ˮ�Ų�һ��
#define SP_E_FLASH_ERASE 0x07  //flash����ʧ��
#define SP_E_FLASH_TRANSDTL_FULL 0x08 //��ˮ�洢����
#define SP_R_PRESS_FUNC 0x09  //���˹��ܼ�����
#define SP_R_BLACKCARD_CONSUME 0x0A  //�ڿ�����
#define SP_E_CARD_STATUS 0x0B  //��״̬����
#define SP_INSUFFICIENT_BALANCE 0x0C  //����
#define SP_R_PRESS_CLEAR 0x0D //�������������
#define SP_E_LESS_THAN_MIN_CARD_BALANCE 0x0E //��Ƭ���С����С�����
#define SP_E_CARD_PASSWD 0x0F //�������������
#define SP_R_CARD_PASSWD_CANCEL 0x10 //ȡ������������
#define SP_E_WRITE_CARD 0x11 //д������
#define SP_E_CALC_MAC 0x12 //����mac����
#define SP_E_NOT_SAME_CARD 0x13 //����ͬһ�ſ�
#define SP_NO_NUMBER_KEY_PRESSED 0x14 //û��������
#define SP_R_CANCEL_CONSUME 0x15 //ȡ������
#define SP_R_CARD_NOT_FOUND 0x16 //Ѱ��ʧ��
#define SP_E_NOT_GET_PROVE 0x17 //û�п۵��������
#define SP_R_HALFWAY_PULL_CARD 0x18 //��;�ο�
#define SP_E_BUFFER_NO_DATA 0x19 //���ջ�����û����
#define SP_E_BUFFER_CRC_ERROR 0x1A //���ջ�����crc����
#define SP_R_WAIT_TIMEOUT 0x1B //�ȴ���Ӧ��ʱ
#define SP_E_SEQNO_NOT_MATCH 0x1C //���յ������˳���
#define SP_E_CMD_NOT_MATCH 0x1D //���յ����������벻һ��
#define SP_E_SERVER 0x1E //ǰ�û��յ����ݺ󷵻ش�����ʱֻ��һ�����ܺ�
#define SP_E_DATE_REVERSE 0x1F //���ڵ���
#define SP_R_DATE_CHANGE 0x20 //���ڱ仯
#define SP_E_BUFFER_SIZE_OUT_OF_RANGE 0x21 //���͵����ݳ��ȳ�����󳤶�
#define SP_E_EXCESSIVE_CALL 0x22 //δ�յ���̨Ӧ���͵ڶ�������
#define SP_E_CMD_NOT_FOUND 0x23 //������δ�ҵ�
#define SP_R_FEERATE_CHANGE 0x24//��Ҫ���·���
#define SP_R_SYSPARA_CHANGE 0x25//��Ҫ����ϵͳ����
#define SP_E_SYS_PASSWD 0x26//�����������
#define SP_E_CARD_DATE_EXPIRE 0x27 //��ʹ�ó�����Ч��
#define SP_E_CARD_PAYCNT_LIMIT 0x28 // ���������ʹ�ô���
#define SP_E_CARD_TYPE_NOT_SUPPORT 0x29 //��Ƭ���֧��
#define SP_S_DEVICEID_NOT_MATCH 0x2A //�ն˱����������
/////////////////////////////////////////////////////////////
#define E_INSUFFICIENT_BALANCE 0x9401




#endif


