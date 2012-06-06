/PROVIDE (__data_start = \.) ;/a \
     PROVIDE (__mi_start = .) ;\
    *(.mi*)\
    KEEP (*(.mi*))\
     PROVIDE (__mi_end = .) ;\
     PROVIDE (__sh_start = .) ;\
    *(.sh*)\
    KEEP (*(.sh*))\
     PROVIDE (__sh_end = .) ;
