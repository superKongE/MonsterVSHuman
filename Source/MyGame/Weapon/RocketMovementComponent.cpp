// Fill out your copyright notice in the Description page of Project Settings.


#include "RocketMovementComponent.h"

// �ùķ��̼��� ����Ǵ� ���� Block(�浹) �Ǹ� ó���Ѵ�
// �浹 �Ŀ��� �ùķ��̼��� ���� ���� ����
// �浹 �Ŀ��� �ùķ��̼��� �ҷ��� ��� ���������� HandleImpact()�� ȣ���� ���� Deflect�� ��ȯ�Ͽ� HandleDelfection()�� ȣ���Ͽ� multi-bound �� sliding �� �����Ѵ�
// �浹 �Ŀ� �ùķ��̼��� ���� ���� ��� Abort�� ��ȯ�Ѵ�
// EHandleBlockingHitResult : HandleBlockingHit() �� ȣ��� �Ŀ� �ùķ��̼��� ��� ����Ǿ�� �ϴ����� ��Ÿ���� enum
URocketMovementComponent::EHandleBlockingHitResult URocketMovementComponent::HandleBlockingHit(const FHitResult& Hit, float TimeTick, const FVector& MoveDelta, float& SubTickTimeRemaining)
{
	Super::HandleBlockingHit(Hit, TimeTick, MoveDelta, SubTickTimeRemaining);

	return EHandleBlockingHitResult::AdvanceNextSubstep;
}

// �浹�� �ӵ��� ������ ��ġ���� Ȱ��ȭ�� ��� bounce ������ �����ϰ�
// Ȱ��ȭ ���� �ʾҰų� �ӵ��� BounceVelocityStopSimulatingThreshold �̸��� ��� �߻�ü�� �ߴܽ�Ų��
void URocketMovementComponent::HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
{
	// Projectile Movement Component�� �⺻������ ��ü�� ��򰡿� �ε����� ���߰� �ϴ� ����� �ֱ� ������
	// �� ����� ��������
	//Super::HandleImpact(Hit, TimeSlice, MoveDelta);
}