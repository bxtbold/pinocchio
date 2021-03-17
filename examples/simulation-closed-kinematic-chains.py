import pinocchio as pin
import numpy as np
import hppfcl as fcl

from pinocchio.visualize import GepettoVisualizer
# Perform the simulation of a four-bar linkages mechanism

height = 0.1
width = 0.01
radius = 0.05

mass_link_A = 10.
length_link_A = 1.
#shape_link_A = fcl.Box(length_link_A,width,height)
shape_link_A = fcl.Capsule(radius,length_link_A)

mass_link_B = 5.
length_link_B = .6
#shape_link_B = fcl.Box(length_link_B,width,height)
shape_link_B = fcl.Capsule(radius,length_link_B)

inertia_link_A = pin.Inertia.FromBox(mass_link_A,length_link_A,width,height)
placement_center_link_A = pin.SE3.Identity()
placement_center_link_A.translation = pin.XAxis * length_link_A / 2.
placement_shape_A = placement_center_link_A.copy()
placement_shape_A.rotation = pin.Quaternion.FromTwoVectors(pin.ZAxis,pin.XAxis).matrix()

inertia_link_B = pin.Inertia.FromBox(mass_link_B,length_link_B,width,height)
placement_center_link_B = pin.SE3.Identity()
placement_center_link_B.translation = pin.XAxis * length_link_B / 2.
placement_shape_B = placement_center_link_B.copy()
placement_shape_B.rotation = pin.Quaternion.FromTwoVectors(pin.ZAxis,pin.XAxis).matrix()

model = pin.Model()
collision_model = pin.GeometryModel()

RED_COLOR = np.array([1.,0.,0.,1.])
WHITE_COLOR = np.array([1.,1.,1.,1.])

base_joint_id = 0
geom_obj0 = pin.GeometryObject("link_A1",base_joint_id,shape_link_A,pin.SE3(pin.Quaternion.FromTwoVectors(pin.ZAxis,pin.XAxis).matrix(),np.zeros((3))))
geom_obj0.meshColor = WHITE_COLOR
collision_model.addGeometryObject(geom_obj0)

joint1_placement = pin.SE3.Identity()
joint1_placement.translation = pin.XAxis * length_link_A/2. 
joint1_id = model.addJoint(base_joint_id,pin.JointModelRY(),joint1_placement,"link_B1")
model.appendBodyToJoint(joint1_id,inertia_link_B,placement_center_link_B)
geom_obj1 = pin.GeometryObject("link_B1",joint1_id,shape_link_B,placement_shape_B)
geom_obj1.meshColor = RED_COLOR
collision_model.addGeometryObject(geom_obj1)

joint2_placement = pin.SE3.Identity()
joint2_placement.translation = pin.XAxis * length_link_B
joint2_id = model.addJoint(joint1_id,pin.JointModelRY(),joint2_placement,"link_A2")
model.appendBodyToJoint(joint2_id,inertia_link_A,placement_center_link_A)
geom_obj2 = pin.GeometryObject("link_A2",joint2_id,shape_link_A,placement_shape_A)
geom_obj2.meshColor = WHITE_COLOR
collision_model.addGeometryObject(geom_obj2)

joint3_placement = pin.SE3.Identity()
joint3_placement.translation = pin.XAxis * length_link_A
joint3_id = model.addJoint(joint2_id,pin.JointModelRY(),joint3_placement,"link_B2")
model.appendBodyToJoint(joint3_id,inertia_link_B,placement_center_link_B)
geom_obj3 = pin.GeometryObject("link_B2",joint3_id,shape_link_B,placement_shape_B)
geom_obj3.meshColor = RED_COLOR
collision_model.addGeometryObject(geom_obj3)

visual_model = collision_model
viz = GepettoVisualizer(model, collision_model, visual_model)
viz.initViewer()
viz.loadViewerModel("pinocchio")
gui = viz.viewer.gui
window_id = viz.viewer.gui.getWindowID('python-pinocchio')

viz.viewer.gui.setBackgroundColor1(window_id, [1., 1., 1., 1.])
viz.viewer.gui.setBackgroundColor2(window_id, [1., 1., 1., 1.])
#viz.viewer.gui.addFloor('hpp-gui/floor')

#viz.viewer.gui.setScale('hpp-gui/floor', [0.5, 0.5, 0.5])
#viz.viewer.gui.setColor('hpp-gui/floor', [0.7, 0.7, 0.7, 1.])
#viz.viewer.gui.setLightingMode('hpp-gui/floor', 'OFF')


q0 = pin.neutral(model)
viz.display(q0)

data = model.createData()
pin.forwardKinematics(model,data,q0)

# Set the contact constraints
constraint1_joint1_placement = pin.SE3.Identity()
constraint1_joint1_placement.translation = pin.XAxis * length_link_B

constraint1_joint2_placement = pin.SE3.Identity()
constraint1_joint2_placement.translation = - pin.XAxis * length_link_A/2.  

constraint_model = pin.RigidContactModel(pin.ContactType.CONTACT_3D,joint3_id,constraint1_joint1_placement,base_joint_id,constraint1_joint2_placement)
constraint_data = constraint_model.createData()
constraint_dim = constraint_model.size() 

# First, do an inverse kinematics kinematics
rho = 1e-10
mu = 1e-4

q = q0.copy()

y = np.ones((constraint_dim))
data.M = np.eye(model.nv) * rho
kkt_constraint = pin.ContactCholeskyDecomposition(model,[constraint_model])
eps = 1e-10
N = 100
for k in range(N):
    pin.computeJointJacobians(model,data,q)
    kkt_constraint.compute(model,data,[constraint_model],[constraint_data],mu)
    constraint_value = constraint_data.c1Mc2.translation

    J = pin.getFrameJacobian(model,data,constraint_model.joint1_id,constraint_model.joint1_placement,constraint_model.reference_frame)[:3,:]
    primal_feas = np.linalg.norm(constraint_value,np.inf)
    dual_feas = np.linalg.norm(J.T.dot(constraint_value + y),np.inf)
    if primal_feas < eps and dual_feas < eps:
        print("Convergence achieved")
        break
    print("constraint_value:",np.linalg.norm(constraint_value))
    rhs = np.concatenate([-constraint_value - y*mu, np.zeros(model.nv)])

    dz = kkt_constraint.solve(rhs) 
    dy = dz[:constraint_dim]
    dq = dz[constraint_dim:]

    alpha = 1.
    q = pin.integrate(model,q,-alpha*dq)
    y -= alpha*(-dy + y)
    #input()

q_sol = (q[:] + np.pi) % np.pi - np.pi
viz.display(q_sol)

# Perform the simulation
q = q_sol.copy()
v = np.zeros((model.nv))
tau = np.zeros((model.nv))
dt = 5e-3

T_sim = 100000
t = 0
mu_sim = 1e-10
constraint_model.corrector.Kp = 10
constraint_model.corrector.Kd = 2. * np.sqrt(constraint_model.corrector.Kp)
pin.initContactDynamics(model,data,[constraint_model])
import time
while t <= T_sim:
    a = pin.contactDynamics(model,data,q,v,tau,[constraint_model],[constraint_data],mu_sim)
    v += a*dt
    q = pin.integrate(model,q,v*dt)
    viz.display(q)
    time.sleep(dt)
    t += dt
