      _$
,$
- $$f

 Y-:-/; --S


 v

  C(7
2(7' (%(:4L
D4L7 4C4X�     

 o6
6 )<#/
'#/ #&#6'
0'
#1 ''Y)K
3)K+) )C)X09
.093 0'0<	�    	�      	�    	J
J	 =ZM
M @SK
K >X

 D 
# 
$   ;#S
*#S #F#a$K
/$K! $>$R'K
3'K%) 'C'X)
6)
)7 ))Y*7
2*7- *%*:-O
=-O1 -=-R4
B4
5C 44Y	�    		�    	�       	u	Wv 		F�      6
6 )<h
h
 [u6
6 )?a
a Tn6
6 )<

 YK
K >X

 E
!
" U#
%#
& ##u#@
(#@) #8#D#h
+#h) #c#t$9
.$9  $'$<$Y
+$Y") $T$e'>
2'>$ ','A(
4(
&5 ((R(D
3(D() (<(Q)>
2)>* ),)A*
8*
,9 **R*D
3*D.) *<*Q-8
<-80 -&-;0
>0
2? 00=1
@1
4A 1194>
.4>6 4,4A7
E7
8F 77>�      	�    		�	   	 		!
w	!X 		+7*
G7*9) 7!7-	SCS     	6
x	6Y 	-	978
H78: 7/7<#	T#DT ##	?
y	?Z) 	;	E8
I8
;J 88/2	U2EU 229
K9
<L 99>7V7FW 77"9*
G9*=) 9!9-8X8GY 88*98
H98> 9/9<9Z9H[ 992:
M:
?N ::/9*
\9*I) 9%91;
O;
@P ;;L:]:J^ ::+;9
Q;9A ;0;>;_;K` ;;-;H
R;HB) ;@;K<a<Lb <<3=c=Md ==6>e>Nf >>.?g?Oh ??0?,
i?,P ?#?/@j@Qk @@.@*
l@*R @#@-AmASn AA2A-
oA-Tp A#A1CqCUr CC!DsDVt DD%   z &1>JXdoy�������������������������������������	�	�	�	�	�	�
�
�
�
�������������������������������������������������������������������packetfunctions.h __PACKETFUNCTIONS_H opendefs.h openserial.h idmanager.h __IDMANAGER_H SLAVENUMBER ACTION_YES ACTION_NO ACTION_TOGGLE packetfunctions_ip128bToMac64b void packetfunctions_ip128bToMac64b(int *, int *, int *) ip128b int * prefix64btoWrite mac64btoWrite packetfunctions_mac64bToIp128b void packetfunctions_mac64bToIp128b(int *, int *, int *) prefix64b mac64b ip128bToWrite packetfunctions_mac64bToMac16b void packetfunctions_mac64bToMac16b(int *, int *) mac16btoWrite packetfunctions_mac16bToMac64b void packetfunctions_mac16bToMac64b(int *, int *) mac16b packetfunctions_isBroadcastMulticast int packetfunctions_isBroadcastMulticast(int *) packetfunctions_isAllRoutersMulticast int packetfunctions_isAllRoutersMulticast(int *) packetfunctions_isAllHostsMulticast int packetfunctions_isAllHostsMulticast(int *) packetfunctions_sameAddress int packetfunctions_sameAddress(int *, int *) packetfunctions_isLinkLocal int packetfunctions_isLinkLocal(int *) packetfunctions_readAddress void packetfunctions_readAddress(int *, int, int *, int) payload type int writeToAddress littleEndian packetfunctions_writeAddress void packetfunctions_writeAddress(int *, int *, int) msg address packetfunctions_reserveHeaderSize void packetfunctions_reserveHeaderSize(int *, int) pkt header_length packetfunctions_tossHeader void packetfunctions_tossHeader(int *, int) packetfunctions_reserveFooterSize void packetfunctions_reserveFooterSize(int *, int) packetfunctions_tossFooter void packetfunctions_tossFooter(int *, int) packetfunctions_duplicatePacket void packetfunctions_duplicatePacket(int *, int *) dst src packetfunctions_calculateCRC void packetfunctions_calculateCRC(int *) packetfunctions_checkCRC int packetfunctions_checkCRC(int *) packetfunctions_calculateChecksum void packetfunctions_calculateChecksum(int *, int *) checksum_ptr packetfunctions_htons void packetfunctions_htons(int, int *) val dest packetfunctions_ntohs int packetfunctions_ntohs(int *) packetfunctions_htonl void packetfunctions_htonl(int, int *) packetfunctions_ntohl int packetfunctions_ntohl(int *) packetfunctions_reverseArrayByteOrder void packetfunctions_reverseArrayByteOrder(int *, int) start len linklocalprefix debugIDManagerEntry_t idmanager_vars_t idmanager_init void idmanager_init(void) idmanager_getIsDAGroot int idmanager_getIsDAGroot(void) idmanager_setIsDAGroot void idmanager_setIsDAGroot(int) newRole idmanager_getIsSlotSkip int idmanager_getIsSlotSkip(void) idmanager_getMyID int * idmanager_getMyID(int) idmanager_setMyID int idmanager_setMyID(int *) idmanager_isMyAddress int idmanager_isMyAddress(int *) idmanager_triggerAboutRoot void idmanager_triggerAboutRoot(void) idmanager_setJoinKey void idmanager_setJoinKey(int *) key idmanager_setJoinAsn void idmanager_setJoinAsn(int *) asn idmanager_getJoinKey void idmanager_getJoinKey(int **) pKey int ** debugPrint_id int debugPrint_id(void) debugPrint_joined int debugPrint_joined(void) onesComplementSum void onesComplementSum(int *, int *, int) global_sum ptr length    [ 2W{��������������������	�	�	�
�
�
������������������������������������������������������������� �  c:packetfunctions.h@37@macro@__PACKETFUNCTIONS_H c:idmanager.h@31@macro@__IDMANAGER_H c:idmanager.h@209@macro@SLAVENUMBER c:idmanager.h@325@macro@ACTION_YES c:idmanager.h@354@macro@ACTION_NO c:idmanager.h@383@macro@ACTION_TOGGLE c:@F@packetfunctions_ip128bToMac64b c:packetfunctions.h@555@F@packetfunctions_ip128bToMac64b@ip128b c:packetfunctions.h@575@F@packetfunctions_ip128bToMac64b@prefix64btoWrite c:packetfunctions.h@605@F@packetfunctions_ip128bToMac64b@mac64btoWrite c:@F@packetfunctions_mac64bToIp128b c:packetfunctions.h@675@F@packetfunctions_mac64bToIp128b@prefix64b c:packetfunctions.h@698@F@packetfunctions_mac64bToIp128b@mac64b c:packetfunctions.h@718@F@packetfunctions_mac64bToIp128b@ip128bToWrite c:@F@packetfunctions_mac64bToMac16b c:packetfunctions.h@788@F@packetfunctions_mac64bToMac16b@mac64b c:packetfunctions.h@809@F@packetfunctions_mac64bToMac16b@mac16btoWrite c:@F@packetfunctions_mac16bToMac64b c:packetfunctions.h@879@F@packetfunctions_mac16bToMac64b@mac16b c:packetfunctions.h@900@F@packetfunctions_mac16bToMac64b@mac64btoWrite c:@F@packetfunctions_isBroadcastMulticast c:@F@packetfunctions_isAllRoutersMulticast c:@F@packetfunctions_isAllHostsMulticast c:@F@packetfunctions_sameAddress c:@F@packetfunctions_isLinkLocal c:@F@packetfunctions_readAddress c:packetfunctions.h@1394@F@packetfunctions_readAddress@payload c:packetfunctions.h@1412@F@packetfunctions_readAddress@type c:packetfunctions.h@1426@F@packetfunctions_readAddress@writeToAddress c:packetfunctions.h@1455@F@packetfunctions_readAddress@littleEndian c:@F@packetfunctions_writeAddress c:packetfunctions.h@1514@F@packetfunctions_writeAddress@msg c:packetfunctions.h@1537@F@packetfunctions_writeAddress@address c:packetfunctions.h@1559@F@packetfunctions_writeAddress@littleEndian c:@F@packetfunctions_reserveHeaderSize c:packetfunctions.h@1667@F@packetfunctions_reserveHeaderSize@pkt c:packetfunctions.h@1690@F@packetfunctions_reserveHeaderSize@header_length c:@F@packetfunctions_tossHeader c:packetfunctions.h@1751@F@packetfunctions_tossHeader@pkt c:packetfunctions.h@1774@F@packetfunctions_tossHeader@header_length c:@F@packetfunctions_reserveFooterSize c:packetfunctions.h@1842@F@packetfunctions_reserveFooterSize@pkt c:packetfunctions.h@1865@F@packetfunctions_reserveFooterSize@header_length c:@F@packetfunctions_tossFooter c:packetfunctions.h@1926@F@packetfunctions_tossFooter@pkt c:packetfunctions.h@1949@F@packetfunctions_tossFooter@header_length c:@F@packetfunctions_duplicatePacket c:packetfunctions.h@2036@F@packetfunctions_duplicatePacket@dst c:packetfunctions.h@2059@F@packetfunctions_duplicatePacket@src c:@F@packetfunctions_calculateCRC c:packetfunctions.h@2142@F@packetfunctions_calculateCRC@msg c:@F@packetfunctions_checkCRC c:@F@packetfunctions_calculateChecksum c:packetfunctions.h@2294@F@packetfunctions_calculateChecksum@msg c:packetfunctions.h@2317@F@packetfunctions_calculateChecksum@checksum_ptr c:@F@packetfunctions_htons c:packetfunctions.h@2391@F@packetfunctions_htons@val c:packetfunctions.h@2405@F@packetfunctions_htons@dest c:@F@packetfunctions_ntohs c:@F@packetfunctions_htonl c:packetfunctions.h@2504@F@packetfunctions_htonl@val c:packetfunctions.h@2518@F@packetfunctions_htonl@dest c:@F@packetfunctions_ntohl c:@F@packetfunctions_reverseArrayByteOrder c:packetfunctions.h@2632@F@packetfunctions_reverseArrayByteOrder@start c:packetfunctions.h@2648@F@packetfunctions_reverseArrayByteOrder@len c:idmanager.h@linklocalprefix c:@debugIDManagerEntry_t c:@idmanager_vars_t c:@F@idmanager_init c:@F@idmanager_getIsDAGroot c:@F@idmanager_setIsDAGroot c:idmanager.h@1363@F@idmanager_setIsDAGroot@newRole c:@F@idmanager_getIsSlotSkip c:@F@idmanager_getMyID c:@F@idmanager_setMyID c:@F@idmanager_isMyAddress c:@F@idmanager_triggerAboutRoot c:@F@idmanager_setJoinKey c:idmanager.h@1662@F@idmanager_setJoinKey@key c:@F@idmanager_setJoinAsn c:idmanager.h@1712@F@idmanager_setJoinAsn@asn c:@F@idmanager_getJoinKey c:idmanager.h@1760@F@idmanager_getJoinKey@pKey c:@F@debugPrint_id c:@F@debugPrint_joined c:@F@onesComplementSum c:packetfunctions.c@270@F@onesComplementSum@global_sum c:packetfunctions.c@291@F@onesComplementSum@ptr c:packetfunctions.c@305@F@onesComplementSum@length     g��<invalid loc> D:\workspace\1_WIRELESS\2_Progarme\WSPS\openwsn\openstack\cross-layers\packetfunctions.c D:\workspace\1_WIRELESS\2_Progarme\WSPS\openwsn\openstack\cross-layers\packetfunctions.h D:\workspace\1_WIRELESS\2_Progarme\WSPS\openwsn\openstack\cross-layers\idmanager.h 