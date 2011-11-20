@echo off

:: Set this path to the path were proj4.*/src lives:
:: set proj4=./contrib/proj-4.7.0/src
:: Please note that proj-4.7.0 has still an error in pconics, which is
:: solved in the trunk of proj. So take that trunk somewhere.

set proj4=c:\svn\other\osgeo_proj4\src

:: This is the path where Boost.Geometry extensions gis projections proj lives:
set bg_proj=../../../boost/geometry/extensions/gis/projections/proj
:: set bg_proj=t

set converter=release\proj4_to_boost_geometry.exe

%converter% %proj4%/pj_aea.c aea > %bg_proj%/aea.hpp
%converter% %proj4%/pj_aeqd.c aeqd > %bg_proj%/aeqd.hpp
%converter% %proj4%/pj_airy.c airy > %bg_proj%/airy.hpp
%converter% %proj4%/pj_aitoff.c aitoff > %bg_proj%/aitoff.hpp
%converter% %proj4%/pj_august.c august > %bg_proj%/august.hpp
%converter% %proj4%/pj_bacon.c bacon > %bg_proj%/bacon.hpp
%converter% %proj4%/pj_bipc.c bipc > %bg_proj%/bipc.hpp
%converter% %proj4%/pj_boggs.c boggs > %bg_proj%/boggs.hpp
%converter% %proj4%/pj_bonne.c bonne > %bg_proj%/bonne.hpp
%converter% %proj4%/pj_cass.c cass > %bg_proj%/cass.hpp
%converter% %proj4%/pj_cc.c cc > %bg_proj%/cc.hpp
%converter% %proj4%/pj_cea.c cea > %bg_proj%/cea.hpp
%converter% %proj4%/pj_chamb.c chamb > %bg_proj%/chamb.hpp
%converter% %proj4%/pj_collg.c collg > %bg_proj%/collg.hpp
%converter% %proj4%/pj_crast.c crast > %bg_proj%/crast.hpp
%converter% %proj4%/pj_denoy.c denoy > %bg_proj%/denoy.hpp
%converter% %proj4%/pj_eck1.c eck1 > %bg_proj%/eck1.hpp
%converter% %proj4%/pj_eck2.c eck2 > %bg_proj%/eck2.hpp
%converter% %proj4%/pj_eck3.c eck3 > %bg_proj%/eck3.hpp
%converter% %proj4%/pj_eck4.c eck4 > %bg_proj%/eck4.hpp
%converter% %proj4%/pj_eck5.c eck5 > %bg_proj%/eck5.hpp
%converter% %proj4%/pj_eqc.c eqc > %bg_proj%/eqc.hpp
%converter% %proj4%/pj_eqdc.c eqdc > %bg_proj%/eqdc.hpp
%converter% %proj4%/pj_fahey.c fahey > %bg_proj%/fahey.hpp
%converter% %proj4%/pj_fouc_s.c fouc_s > %bg_proj%/fouc_s.hpp
%converter% %proj4%/pj_gall.c gall > %bg_proj%/gall.hpp
%converter% %proj4%/pj_geocent.c geocent > %bg_proj%/geocent.hpp
%converter% %proj4%/pj_geos.c geos > %bg_proj%/geos.hpp
%converter% %proj4%/pj_gins8.c gins8 > %bg_proj%/gins8.hpp
%converter% %proj4%/pj_gn_sinu.c gn_sinu > %bg_proj%/gn_sinu.hpp
%converter% %proj4%/pj_gnom.c gnom > %bg_proj%/gnom.hpp
%converter% %proj4%/pj_goode.c goode > %bg_proj%/goode.hpp
%converter% %proj4%/pj_gstmerc.c gstmerc > %bg_proj%/gstmerc.hpp
%converter% %proj4%/pj_hammer.c hammer > %bg_proj%/hammer.hpp
%converter% %proj4%/pj_hatano.c hatano > %bg_proj%/hatano.hpp
%converter% %proj4%/pj_imw_p.c imw_p > %bg_proj%/imw_p.hpp
%converter% %proj4%/pj_krovak.c krovak > %bg_proj%/krovak.hpp
%converter% %proj4%/pj_labrd.c labrd > %bg_proj%/labrd.hpp
%converter% %proj4%/pj_laea.c laea > %bg_proj%/laea.hpp
%converter% %proj4%/pj_lagrng.c lagrng > %bg_proj%/lagrng.hpp
%converter% %proj4%/pj_larr.c larr > %bg_proj%/larr.hpp
%converter% %proj4%/pj_lask.c lask > %bg_proj%/lask.hpp
%converter% %proj4%/pj_latlong.c latlong > %bg_proj%/latlong.hpp
%converter% %proj4%/pj_lcc.c lcc > %bg_proj%/lcc.hpp
%converter% %proj4%/pj_lcca.c lcca > %bg_proj%/lcca.hpp
%converter% %proj4%/pj_loxim.c loxim > %bg_proj%/loxim.hpp
%converter% %proj4%/pj_lsat.c lsat > %bg_proj%/lsat.hpp
%converter% %proj4%/pj_mbt_fps.c mbt_fps > %bg_proj%/mbt_fps.hpp
%converter% %proj4%/pj_mbtfpp.c mbtfpp > %bg_proj%/mbtfpp.hpp
%converter% %proj4%/pj_mbtfpq.c mbtfpq > %bg_proj%/mbtfpq.hpp
%converter% %proj4%/pj_merc.c merc > %bg_proj%/merc.hpp
%converter% %proj4%/pj_mill.c mill > %bg_proj%/mill.hpp
%converter% %proj4%/pj_mod_ster.c mod_ster > %bg_proj%/mod_ster.hpp
%converter% %proj4%/pj_moll.c moll > %bg_proj%/moll.hpp
%converter% %proj4%/pj_nell.c nell > %bg_proj%/nell.hpp
%converter% %proj4%/pj_nell_h.c nell_h > %bg_proj%/nell_h.hpp
%converter% %proj4%/pj_nocol.c nocol > %bg_proj%/nocol.hpp
%converter% %proj4%/pj_nsper.c nsper > %bg_proj%/nsper.hpp
%converter% %proj4%/pj_nzmg.c nzmg > %bg_proj%/nzmg.hpp
%converter% %proj4%/pj_ob_tran.c ob_tran > %bg_proj%/ob_tran.hpp
%converter% %proj4%/pj_ocea.c ocea > %bg_proj%/ocea.hpp
%converter% %proj4%/pj_oea.c oea > %bg_proj%/oea.hpp
%converter% %proj4%/pj_omerc.c omerc > %bg_proj%/omerc.hpp
%converter% %proj4%/pj_ortho.c ortho > %bg_proj%/ortho.hpp
%converter% %proj4%/pj_poly.c poly > %bg_proj%/poly.hpp
%converter% %proj4%/pj_putp2.c putp2 > %bg_proj%/putp2.hpp
%converter% %proj4%/pj_putp3.c putp3 > %bg_proj%/putp3.hpp
%converter% %proj4%/pj_putp4p.c putp4p > %bg_proj%/putp4p.hpp
%converter% %proj4%/pj_putp5.c putp5 > %bg_proj%/putp5.hpp
%converter% %proj4%/pj_putp6.c putp6 > %bg_proj%/putp6.hpp
%converter% %proj4%/pj_robin.c robin > %bg_proj%/robin.hpp
%converter% %proj4%/pj_rpoly.c rpoly > %bg_proj%/rpoly.hpp
%converter% %proj4%/pj_sconics.c sconics > %bg_proj%/sconics.hpp
%converter% %proj4%/pj_somerc.c somerc > %bg_proj%/somerc.hpp
%converter% %proj4%/pj_stere.c stere > %bg_proj%/stere.hpp
%converter% %proj4%/pj_sterea.c sterea > %bg_proj%/sterea.hpp
%converter% %proj4%/pj_sts.c sts > %bg_proj%/sts.hpp
%converter% %proj4%/pj_tcc.c tcc > %bg_proj%/tcc.hpp
%converter% %proj4%/pj_tcea.c tcea > %bg_proj%/tcea.hpp
%converter% %proj4%/pj_tmerc.c tmerc > %bg_proj%/tmerc.hpp
%converter% %proj4%/pj_tpeqd.c tpeqd > %bg_proj%/tpeqd.hpp
%converter% %proj4%/pj_urm5.c urm5 > %bg_proj%/urm5.hpp
%converter% %proj4%/pj_urmfps.c urmfps > %bg_proj%/urmfps.hpp
%converter% %proj4%/pj_vandg.c vandg > %bg_proj%/vandg.hpp
%converter% %proj4%/pj_vandg2.c vandg2 > %bg_proj%/vandg2.hpp
%converter% %proj4%/pj_vandg4.c vandg4 > %bg_proj%/vandg4.hpp
%converter% %proj4%/pj_wag2.c wag2 > %bg_proj%/wag2.hpp
%converter% %proj4%/pj_wag3.c wag3 > %bg_proj%/wag3.hpp
%converter% %proj4%/pj_wag7.c wag7 > %bg_proj%/wag7.hpp
%converter% %proj4%/pj_wink1.c wink1 > %bg_proj%/wink1.hpp
%converter% %proj4%/pj_wink2.c wink2 > %bg_proj%/wink2.hpp
%converter% %proj4%/proj_rouss.c rouss > %bg_proj%/rouss.hpp
