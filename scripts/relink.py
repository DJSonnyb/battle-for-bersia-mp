import os as _os_; ROOT = _os_.path.dirname(_os_.path.dirname(_os_.path.abspath(__file__))) + '/'   # repository root
import re,sys
S=ROOT
lst=open(S+'newcode/kuf.lst').read().split('\n')
def lab(name):
    for i,l in enumerate(lst):
        if l.strip().endswith(name+':'):
            for l2 in lst[i+1:i+4]:
                m=re.match(r'\s*\d+\s+([0-9A-F]{8})',l2)
                if m: return int(m.group(1),16)
    raise SystemExit('label not found '+name)
p=S+'out/patch_heroes_multi_lobby.py'; s=open(p).read()
for va,key,kind in ((0x69B693,'enemyfill_stub','call'),(0x69B64B,'snap_stub','call'),(0x69B8F6,'customai_stub','call'),(0x54A499,'inv_reset','call'),(0x6AD300,'inv_set','jmp'),(0x6AD310,'inv_get','jmp'),(0x7289A1,'autocg','jmp'),(0x721C04,'enemychk_stub','call'),(0x6A3A94,'st62','jmp'),(0x6A73A7,'st3auto','jmp'),(0x6A39A1,'rs_stub','call'),(0x71F0B0,'tsel_hook','jmp'),(0x69E64E,'bloopA','call'),(0x69E723,'bloopB','call'),(0x426160,'pause_hook','jmp'),(0x7378CC,'pmenu_hook','call'),(0x6A3077,'mp_direct','jmp'),(0x6A34EE,'mp_direct2','jmp'),(0x69E4DA,'sw_hook','jmp'),(0x6EFDC0,'om_hook','jmp'),(0x5A2CB0,'wg_cb0','jmp'),(0x5A2CE0,'wg_ce0','jmp'),(0x5A2D10,'wg_d10','jmp'),(0x5A2D40,'wg_d40','jmp'),(0x5A2D60,'wg_d60','jmp'),(0x7183A6,'mp_bbld','call'),(0x7051CC,'mp_fill','call'),(0x7031C1,'jc_hook','jmp'),(0x71B280,'cs_save_hook','jmp'),(0x71B6A0,'cs_load_hook','jmp'),(0x4682F0,'in_hook1','jmp'),(0x46A9F0,'in_hook2','jmp'),(0x476824,'tgt_guard','jmp'),(0x46A980,'in_hook3','jmp'),(0x75D630,'kb_hook','jmp'),(0x69B87D,'voice_sel','call'),(0x726210,'slot_sum','jmp'),(0x725EF8,'slot_add','jmp'),(0x6B6D20,'slot_get','jmp'),(0x6FE9A4,'learn_row','jmp'),(0x6FECAB,'learn_list','call'),(0x6FA1D2,'learn_list','call'),(0x6FF6ED,'learn_list','call'),(0x7047AC,'job_pal','jmp'),(0x71E1E9,'cust_build','call'),(0x476C90,'ord_guard','jmp'),(0x4DEA9C,'sp_hook','jmp'),(0x541660,'mc_hook','jmp'),(0x541860,'mf_hook','jmp'),(0x549F60,'amc_hook','jmp'),(0x82A009,'k2a_hook','jmp'),(0x83FE89,'fx_guard','jmp'),(0x6906B0,'lk_wrap','jmp'),(0x55E681,'exp_net','call'),(0x463F57,'exp_net','call'),(0x463F44,'exp_disp','call'),(0x772C10,'bgm_hook','jmp'),(0x69B5A5,'ls_lp','jmp'),(0x7572C2,'lose_hook','jmp'),(0x6B3653,'live_sw','call'),(0x6B92A5,'live_sw','call'),(0x6BC923,'live_sw','call'),(0x5867DE,'tk_dmg','jmp'),(0x54AAB5,'iv_guard','jmp'),(0x54AB7D,'iv_guard2','jmp'),(0x54AA96,'iv_hnull','jmp'),(0x7AE071,'sox_open','call'),(0x7AE143,'sox_open','call'),(0x583500,'race_map','jmp'),(0x583F60,'type_map','jmp'),(0x6E19F0,'uvid_map','jmp'),(0x6E17AF,'tree_ext','call'),(0x6DF164,'enca_tr','jmp'),(0x560470,'afw_use','jmp'),(0x557EEB,'pis_guard','jmp'),(0x49D250,'afw_eng','jmp'),(0x87D6FA,'tex_rd','call'),(0x4509BA,'sfont_add','call'),(0x6DFCC4,'nav_id','jmp'),(0x51A7AD,'stg_open','call'),(0x6A0925,'rng2_hook','call'),(0x488B30,'rng2_hk','jmp'),(0x55D354,'ogre_var','call'),(0x56AE9A,'lives_hk','call'),(0x55D4EA,'wpn_nostat','call'),(0x87AA56,'snd_guard','jmp'),(0x5867E6,'dmg_hk','jmp'),(0x4990EC,'heal_hk','jmp'),(0x87A1A0,'snd_id1','jmp'),(0x87A9B0,'snd_id2','jmp'),(0x5410C9,'lp_script','call'),(0x54110E,'lp_script','call'),(0x541125,'lp_script','call'),(0x541769,'lp_script','call'),(0x54192E,'lp_script','call'),(0x541957,'lp_script','call'),(0x54213C,'lp_script','call'),(0x542153,'lp_script','call'),(0x5421EF,'lp_script','call'),(0x542239,'lp_script','call'),(0x542250,'lp_script','call'),(0x544986,'lp_script','call'),(0x5449B9,'lp_script','call'),(0x5455D9,'lp_script','call'),(0x5460DB,'lp_script','call'),(0x5460F3,'lp_script','call'),(0x54615B,'lp_script','call'),(0x546173,'lp_script','call'),(0x5468FC,'lp_script','call'),(0x54755F,'lp_script','call'),(0x54761F,'lp_script','call'),(0x5476DF,'lp_script','call'),(0x54775A,'lp_script','call'),(0x547E3F,'lp_script','call'),(0x547EAF,'lp_script','call'),(0x547F0F,'lp_script','call'),(0x54804D,'lp_script','call'),(0x548369,'lp_script','call'),(0x548EAF,'lp_script','call'),(0x548ECC,'lp_script','call'),(0x54912C,'lp_script','call'),(0x54972F,'lp_script','call'),(0x549753,'lp_script','call'),(0x54A08F,'lp_script','call'),(0x54A0AC,'lp_script','call'),(0x54A16F,'lp_script','call'),(0x54A18C,'lp_script','call'),(0x54AD4C,'lp_script','call'),(0x55700F,'lp_script','call'),(0x557CA0,'lp_script','call'),(0x557EAF,'lp_script','call'),(0x557F2D,'lp_script','call'),(0x55984F,'lp_script','call')):
    s,n=re.subn(r"%s\(0x%X, KUF_VA \+ 0x[0-9A-Fa-f]+\)"%(kind,va), "%s(0x%X, KUF_VA + 0x%X)"%(kind,va,lab(key)), s); assert n==1,hex(va)
s,n=re.subn(r"struct\.pack\('<I', KUF_VA \+ 0x[0-9A-Fa-f]+\)\)\)", "struct.pack('<I', KUF_VA + 0x%X)))"%lab('troopdone'), s); assert n==1
for fixed,off in (('ctor_hook',0),('state5',0x400),('getdir',0xE00),('sel_stub',0xE20),('slotfix',0xF00),('armyfix',0xF40)):
    assert lab(fixed)==off,(fixed,hex(lab(fixed)))
for _nm,_lb in (('IVWAIT_VA','iv_wait_vt'),('LS_VT0_VA','ls_vt0'),('LS_VT1_VA','ls_vt1'),('LIVE_T_VA','live_t')):
    s,n=re.subn(_nm+r" = 0x[0-9A-Fa-f]+", "%s = 0x%X"%(_nm,0xAE1000+lab(_lb)), s); assert n==1,_nm
s,n=re.subn(r"HEROTAB_VA = 0x[0-9A-Fa-f]+", "HEROTAB_VA = 0x%X"%(0xAE1000+lab('herotab13')), s); assert n==1
for _nm,_lb in (('JOBSOLD_VA','jobsold'),('VOICETAB_VA','voicetab'),('SAYW0_VA','say_w0'),('SAYW1_VA','say_w1'),('SAYW2_VA','say_w2'),('SAYW3_VA','say_w3')):
    s,n=re.subn(_nm+r" = 0x[0-9A-Fa-f]+", "%s = 0x%X"%(_nm,0xAE1000+lab(_lb)), s); assert n==1,_nm
blob=open(S+'newcode/kuf.bin','rb').read()
assert len(blob)<0x8000
nb=open(S+'net/net.bin','rb').read()
blob=blob+b'\0'*(0x10000-len(blob))+nb
h=blob.hex(); lines='\n'.join("    '%s'"%h[i:i+96] for i in range(0,len(h),96))
s=re.sub(r"KUF_BLOB = bytes\.fromhex\(\n.*?\n\)\n", "KUF_BLOB = bytes.fromhex(\n"+lines+"\n)\n", s, count=1, flags=re.S)
jb=open(S+'jobs/build/jobdata.bin','rb').read(); assert len(jb)<=0x3000
jb=jb+b'\0'*(0x3000-len(jb))+open(S+'net/netdata.bin','rb').read(); assert len(jb)<=0x7000
jb=jb+b'\0'*(0x9000-len(jb))+open(S+'net/ktext.bin','rb').read(); hj=jb.hex()   # v99: overflow code at .kjb+0x9000 (0xB1C000)   # v95: net .rodata/.data at .kjb+0x3000 (0xB16000)
s,n=re.subn(r"JOB_BLOB = bytes\.fromhex\(\n.*?\n\)\n", lambda m: "JOB_BLOB = bytes.fromhex(\n"+'\n'.join("    '%s'"%hj[i:i+96] for i in range(0,len(hj),96))+"\n)\n", s, count=1, flags=re.S); assert n==1
open(p,'w').write(s); print('relinked, code size',hex(len(blob)))
