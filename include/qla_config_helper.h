#ifndef _QLA_CONFIG_HELPER_H
#define _QLA_CONFIG_HELPER_H

#ifdef __cplusplus
extern "C" {
#endif


  // QLA_Colors == 0 is an alias for QLA_Colors set to 'N',
  // but it is easier to pass just 0 via make.
#ifdef QLA_Colors
#if QLA_Colors == 0
#undef QLA_Colors
#define QLA_Colors 'N'
#endif
#endif


#ifdef __cplusplus
}
#endif

#endif /* _QLA_CONFIG_HELPER_H */
