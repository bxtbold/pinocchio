//
// Copyright (c) 2019-2020 INRIA, CNRS
//

#ifndef __pinocchio_algorithm_contact_info_hpp__
#define __pinocchio_algorithm_contact_info_hpp__

#include "pinocchio/multibody/fwd.hpp"
#include "pinocchio/spatial/se3.hpp"
#include "pinocchio/spatial/motion.hpp"
#include "pinocchio/spatial/force.hpp"

#include <limits>

namespace pinocchio
{
  /// \brief Type of contact
  enum ContactType
  {
    CONTACT_3D = 0,       /// \brief Point contact model
    CONTACT_6D,           /// \brief Frame contact model
    CONTACT_UNDEFINED     /// \brief The default contact is undefined
  };
  
  template<ContactType contact_type>
  struct contact_dim
  {
    enum { value = 0 };
  };
  
  template<>
  struct contact_dim<CONTACT_3D>
  {
    enum { value  = 3 };
  };
  
  template<>
  struct contact_dim<CONTACT_6D>
  {
    enum { value  = 6 };
  };

  template<typename Scalar, int Options> struct RigidContactModelTpl;
  template<typename Scalar, int Options> struct RigidContactDataTpl;
  
  ///
  /// \brief Contact model structure containg all the info describing the rigid contact model
  ///
  template<typename _Scalar, int _Options>
  struct RigidContactModelTpl
  {
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    
    typedef _Scalar Scalar;
    enum { Options = _Options };
    
    typedef RigidContactModelTpl ContactModel;
    typedef RigidContactDataTpl<Scalar,Options> ContactData;
    
    typedef SE3Tpl<Scalar,Options> SE3;
    typedef MotionTpl<Scalar,Options> Motion;
    typedef ForceTpl<Scalar,Options> Force;
    typedef pinocchio::FrameIndex FrameIndex;
    
    /// \brief Type of the contact.
    ContactType type;
    
    /// \brief Index of the parent Frame in the model tree.
    FrameIndex frame_id;
    
    /// \brief Reference frame where the constraint is expressed (WORLD, LOCAL_WORLD_ALIGNED or LOCAL)
    ReferenceFrame reference_frame;
    
    /// \brief Desired contact placement
    SE3 desired_contact_placement;
    
    /// \brief Desired contact spatial velocity
    Motion desired_contact_velocity;
    
    /// \brief Desired contact spatial acceleration
    Motion desired_contact_acceleration;
    
    /// \brief Default constructor.
    RigidContactModelTpl()
    : type(CONTACT_UNDEFINED)
    , frame_id(std::numeric_limits<FrameIndex>::max())
    , reference_frame(WORLD)
    , desired_contact_placement(SE3::Identity())
    , desired_contact_velocity(Motion::Zero())
    , desired_contact_acceleration(Motion::Zero())
    {}
        
    ///
    /// \brief Contructor with from a given type and parent
    ///
    /// \param[in] type Type of the contact.
    /// \param[in] frame_id Index of the parent Frame in the model tree.
    ///
    RigidContactModelTpl(const ContactType type,
                         const FrameIndex frame_id,
                         const ReferenceFrame & reference_frame = WORLD)
    : type(type)
    , frame_id(frame_id)
    , reference_frame(reference_frame)
    , desired_contact_placement(SE3::Identity())
    , desired_contact_velocity(Motion::Zero())
    , desired_contact_acceleration(Motion::Zero())
    {}
    
    ///
    /// \brief Comparison operator
    ///
    /// \param[in] other Other RigidContactModelTpl to compare with.
    ///
    /// \returns true if the two *this is equal to other (type and parent attributs must be the same).
    ///
    template<int OtherOptions>
    bool operator==(const RigidContactModelTpl<Scalar,OtherOptions> & other) const
    {
      return
         type == other.type
      && frame_id == other.frame_id
      && reference_frame == other.reference_frame;
    }
    
    ///
    /// \brief Oposite of the comparison operator.
    ///
    /// \param[in] other Other RigidContactModelTpl to compare with.
    ///
    /// \returns false if the two *this is not equal to other (at least type and parent attributs is different).
    ///
    template<int OtherOptions>
    bool operator!=(const RigidContactModelTpl<Scalar,OtherOptions> & other) const
    {
      return !(*this == other);
    }
    
    int size() const
    {
      switch(type)
      {
        case CONTACT_3D:
          return contact_dim<CONTACT_3D>::value;
        case CONTACT_6D:
          return contact_dim<CONTACT_6D>::value;
        default:
          return contact_dim<CONTACT_UNDEFINED>::value;
      }
      return -1;
    }

    /// \returns An expression of *this with the Scalar type casted to NewScalar.
    template<typename NewScalar>
    RigidContactModelTpl<NewScalar,Options> cast() const
    {
      typedef RigidContactModelTpl<NewScalar,Options> ReturnType;
      ReturnType res;
      res.type = type;
      res.frame_id = frame_id;
      res.reference_frame =reference_frame;
      res.desired_contact_placement = desired_contact_placement.template cast<NewScalar>();
      res.desired_contact_velocity = desired_contact_velocity.template cast<NewScalar>();
      res.desired_contact_acceleration = desired_contact_acceleration.template cast<NewScalar>();
      return res;
    }
    
  };

  ///
  /// \brief Contact model structure containg all the info describing the rigid contact model
  ///
  template<typename _Scalar, int _Options>
  struct RigidContactDataTpl
  {
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    
    typedef _Scalar Scalar;
    enum { Options = _Options };
    
    typedef RigidContactModelTpl<Scalar,Options> ContactModel;
    typedef RigidContactDataTpl ContactData;
    
    typedef SE3Tpl<Scalar,Options> SE3;
    typedef MotionTpl<Scalar,Options> Motion;
    typedef ForceTpl<Scalar,Options> Force;
    
    RigidContactDataTpl(const ContactModel & /*contact_model*/)
    : contact_force(Force::Zero())
    , contact_velocity(Motion::Zero())
    , contact_acceleration(Motion::Zero())
    , contact_acceleration_drift(Motion::Zero())
    , contact_acceleration_deviation(Motion::Zero())
    {}
    
    // data
    
    /// \brief Resulting contact forces
    Force contact_force;

    /// \brief Current contact spatial velocity
    Motion contact_velocity;
    
    /// \brief Current contact spatial acceleration
    Motion contact_acceleration;
    
    /// \brief Current contact drift acceleration (acceleration only due to the Coriolis and centrifugal effects).
    Motion contact_acceleration_drift;
    
    /// \brief Contact deviation from the reference acceleration (a.k.a the error)
    Motion contact_acceleration_deviation;
    
    bool operator==(const RigidContactDataTpl & other) const
    {
      return
         contact_force == other.contact_force
      && contact_velocity == other.contact_velocity
      && contact_acceleration == other.contact_acceleration
      && contact_acceleration_drift == other.contact_acceleration_drift
      && contact_acceleration_deviation == other.contact_acceleration_deviation
      ;
    }
    
    bool operator!=(const RigidContactDataTpl & other) const
    {
      return !(*this == other);
    }
  };
  
  typedef RigidContactModelTpl<double,0> RigidContactModel;
  typedef RigidContactDataTpl<double,0> RigidContactData;
}

#endif // ifndef __pinocchio_algorithm_contact_info_hpp__
