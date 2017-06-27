#include "AnimatNode.hpp"
#include "../ModelLoader.hpp"

void AnimatNode::draw()
{
	if (timeAccumulator > 0.03)
	{
		Animator* animator = ModelLoader::getInstance()->animator;
		animator->UpdateAnimation(animationTime += 0.001, animator->ANIMATION_TICKS_PER_SECOND);// Argument is here the time inside the animation, not time delta!
		timeAccumulator = 0.0;
		
		moveEnemy();
	
		
	
	}
	
	ModelNode::draw();
	
}

void AnimatNode::moveEnemy()
{
	/*
	glm::vec3 walkPoint1 = glm::vec3(-5.7f, -0.3275f, 12.0f);
	glm::vec3 walkPoint2 = glm::vec3(20.5f, -0.3275f, 12.0f);
	glm::vec3 walkPoint3 = glm::vec3(20.5f, -0.3275f, -13.75f);
	glm::vec3 walkPoint4 = glm::vec3(-5.7f, -0.3275f, -13.75f);
	*/
	glm::vec3 positionEnemy = ModelNode::hirachicalModelMatrix[3];
	TransformationNode* dn = dynamic_cast<TransformationNode*>(this);
	if(!endOf1Reached)
	{
		//positionEnemy.z = positionEnemy.z + 0.05;	
		this->rigidBody->translate(btVector3(0, 0, MOVESTEP)); //move her around the floor //rigidbody-translate is always relativ to itself
		if (positionEnemy.z >= walkPoint1.z) //take care of +/- !
		{
			endOf1Reached = true;
			endOf4Reached = false;
			setBulletRotation(90.0f);
		}
	}
	else if (!endOf2Reached) 
	{
		//positionEnemy.x = positionEnemy.x + 0.05;
		this->rigidBody->translate(btVector3(MOVESTEP, 0, 0)); //move her around the floor
		if (positionEnemy.x >= walkPoint2.x) //take care of +/- !
		{
			endOf2Reached = true;
			setBulletRotation(180.0f);
		}
	}
	else if (!endOf3Reached)
	{
		//positionEnemy.z = positionEnemy.z - 0.05;
		this->rigidBody->translate(btVector3(0, 0, -MOVESTEP)); //move her around the floor
		if (positionEnemy.z <= walkPoint3.z) //take care of +/- !
		{
			endOf3Reached = true;
			setBulletRotation(270.0f);
		}
	}
	else //if (!endOf4Reached) we never need this
	{
		//positionEnemy.x = positionEnemy.x - 0.05;
		this->rigidBody->translate(btVector3(-MOVESTEP,0,0)); //move her around the floor
		if (positionEnemy.x <= walkPoint4.x) //take care of +/- !
		{ //reset the cycle, but keep sightpoint with endOf4Reached active!
			endOf4Reached = true;
			endOf1Reached = false;
			endOf2Reached = false;
			endOf3Reached = false;		
			setBulletRotation(0.0f);
		}
	}
}

void AnimatNode::setBulletRotation(float finalRotateInDegree)
{
	glm::vec3 positionEnemy = ModelNode::hirachicalModelMatrix[3];
	btTransform trans(btQuaternion(btVector3(0, 1, 0), btRadians(finalRotateInDegree)), btVector3(positionEnemy.x, positionEnemy.y, positionEnemy.z));
	trans.setIdentity();
	trans.setRotation(btQuaternion(btVector3(0, 1, 0), btRadians(finalRotateInDegree)));
	this->rigidBody->setCenterOfMassTransform(trans);
	this->rigidBody->setWorldTransform(trans);
	this->rigidBody->translate(btVector3(positionEnemy.x, positionEnemy.y, positionEnemy.z)); //put her into the floor
}