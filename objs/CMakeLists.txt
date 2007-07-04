set(s3d-objs abs2.3ds abs.3ds accesspoint.3ds accesspoint_inet.3ds arrow2.3ds
	arrow.3ds AUTHORS big_p.3ds bigsphere.3ds btn_close.3ds btn_maximize.3ds
	btn_minimize.3ds btn_rotate.3ds clock_bg.3ds comp.3ds cube.3ds cubey.3ds
	cubeyholes.3ds dot.3ds dotdot.3ds duno.3ds film.3ds folder.3ds
	geometry.3ds go_button.3ds internet.3ds katze_body.3ds katze_leg.3ds
	katze_tail.3ds lil_p.3ds materials.3ds meshnode.3ds miu.3ds
	new_sphere.3ds notes.3ds reset.3ds ringsystem.3ds rotate.3ds
	s3d_berlios_de.3ds s3dstart.3ds sec_p.3ds snow_body.3ds snow_foot.3ds
	snow_head.3ds sphere.3ds spikeybla.3ds star.3ds weirdball.3ds
	wtf.3ds xyz.3ds)

# install
install(FILES ${s3d-objs} DESTINATION share/s3d/objs)
