#! /bin/sh -e
for man in dot_mcp.1  kism3d.1  meshs3d.1  s3d.1  s3dfm.1  s3dosm.1  s3dvt.1  s3d_x11gate.1; \
               do docbook-to-man ${man%??}manpage.sgml >$man; \
done
