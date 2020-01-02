/*
 * This file is part of WEIBO AD RANKING
 *
 * Any question contact with weibo ad department support
 */

#ifndef SERVICE_H
#define SERVICE_H

namespace rank {

/**
 * This class simplifies use of Services classes.
 *
 * For example, the following code:
 * \code
 *   Service<ConfigurationSettings> pServices;
 *   bool bHasKey = pServices->hasKey("Key");
 * \endcode
 *
 * The following can also be done:
 * \code
 *   bool bHasKey = Service<ConfigurationSettings>()->hasKey("Key");
 * \endcode
 *
 */
template <class T>
class Service {
 public:
  /**
  * Provides direct access to the Service pointer.
  *
  * @return A pointer of type T. No implementation is provided in order to
  *         force specialization. Specialized methods need to ensure that
  *         NULL will never be returned.
  */
  T *get() const;

  /**
  * Allows the held Service pointer to be used with pointer indirection.
  *
  * @return A pointer of type T; for all template specializations,
  *         this is guaranteed to be non-null.
  */
  T *operator->() const { return get(); }
};

}  // namespace rank

#endif
