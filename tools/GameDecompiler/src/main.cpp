/////////////////////////////////////////
//
//         LieroX Game Script Compiler
//
//     Copyright Auxiliary Software 2002
//
//
/////////////////////////////////////////


// Main compiler
// Created 7/2/02
// Jason Boettcher


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "CVec.h"
#include "CGameScript.h"
#include "ConfigHandler.h"


CGameScript	*Game;



int		Decompile();
int		DecompileWeapon(int id);
void	DecompileBeam(FILE * fp, weapon_t *Weap);
void	DecompileProjectile(proj_t * proj, const char * weaponName);
int		DecompileExtra(FILE * fp);

int		DecompileJetpack(FILE * fp, weapon_t *Weap);

int ProjCount = 0;


///////////////////
// Main entry point
int main(int argc, char *argv[])
{
	printf("Liero Xtreme Game Script Decompiler v0.3\nCopyright OLX team 2009\n");
	printf("GameScript Version: %d\n\n\n",GS_VERSION);


	Game = new CGameScript;
	if(Game == NULL) {
		printf("Error: Out of memory\n");
		return 1;
	}

	// Add some keywords
	AddKeyword("WPN_PROJECTILE",WPN_PROJECTILE);
	AddKeyword("WPN_SPECIAL",WPN_SPECIAL);
	AddKeyword("WPN_BEAM",WPN_BEAM);
	AddKeyword("WCL_AUTOMATIC",WCL_AUTOMATIC);
	AddKeyword("WCL_POWERGUN",WCL_POWERGUN);
	AddKeyword("WCL_GRENADE",WCL_GRENADE);
	AddKeyword("WCL_CLOSERANGE",WCL_CLOSERANGE);
	AddKeyword("PRJ_PIXEL",PRJ_PIXEL);
	AddKeyword("PRJ_IMAGE",PRJ_IMAGE);
	AddKeyword("Bounce",PJ_BOUNCE);
	AddKeyword("Explode",PJ_EXPLODE);
	AddKeyword("Injure",PJ_INJURE);
	AddKeyword("Carve",PJ_CARVE);
	AddKeyword("Dirt",PJ_DIRT);
    AddKeyword("GreenDirt",PJ_GREENDIRT);
    AddKeyword("Disappear",PJ_DISAPPEAR);
	AddKeyword("Nothing",PJ_NOTHING);
	AddKeyword("TRL_NONE",TRL_NONE);
	AddKeyword("TRL_SMOKE",TRL_SMOKE);
	AddKeyword("TRL_CHEMSMOKE",TRL_CHEMSMOKE);
	AddKeyword("TRL_PROJECTILE",TRL_PROJECTILE);
	AddKeyword("TRL_DOOMSDAY",TRL_DOOMSDAY);
	AddKeyword("TRL_EXPLOSIVE",TRL_EXPLOSIVE);
	AddKeyword("SPC_JETPACK",SPC_JETPACK);
	AddKeyword("ANI_ONCE",ANI_ONCE);
	AddKeyword("ANI_LOOP",ANI_LOOP);
	AddKeyword("ANI_PINGPONG",ANI_PINGPONG);
	AddKeyword("true",true);
	AddKeyword("false",false);


	char * dir = ".";
	if( argc > 1 )
		dir = argv[1];
	
	if( Game->Load(dir) != GSE_OK )
	{
		printf("\nError loading mod!\n");
		return 1;
	}
	printf("\nInfo:\nWeapons: %d\n",Game->GetNumWeapons());

	Decompile();

	printf("\nDone!\nInfo:\nWeapons: %d Projectiles: %d\n",Game->GetNumWeapons(), ProjCount );
	

	Game->Shutdown();
	if(Game)
		delete Game;

	return 0;
}




int Decompile()
{
	char buf[64],wpn[64],weap[32];
	int num,n;


	// Check the file
	FILE *fp = fopen("Main.txt", "w");
	if(!fp)
		return false;


	fprintf(fp, "\n[General]\n\nModName = %s\n",Game->GetHeader()->ModName);

	fprintf(fp, "\n[Weapons]\n\nNumWeapons = %i\n",Game->GetNumWeapons());


	weapon_t *weapons;

	// Compile the weapons
	for(n=0;n<Game->GetNumWeapons();n++) {
		fprintf(fp,"Weapon%d = w_%s.txt\n", n+1, Game->GetWeapons()[n].Name );
	}

	// Compile the extra stuff
	DecompileExtra(fp);

	fclose(fp);

	for(n=0;n<Game->GetNumWeapons();n++) {
		DecompileWeapon(n);
	}

	return true;
}


///////////////////
// Compile a weapon
int DecompileWeapon(int id)
{
	weapon_t *Weap = Game->GetWeapons()+id;
	
	char fname[128];
	sprintf(fname, "w_%s.txt", Weap->Name);
	
	FILE * fp = fopen(fname, "w");
	if(!fp)
		return false;

	printf("Compiling Weapon '%s'\n",Weap->Name);
	fprintf(fp,"[General]\n\nName = %s\n",Weap->Name);
	fprintf(fp,"Type = %s\n", (
				Weap->Type == WPN_PROJECTILE ? "WPN_PROJECTILE" : (
				Weap->Type == WPN_SPECIAL ? "WPN_SPECIAL" : (
				Weap->Type == WPN_BEAM ? "WPN_BEAM" : "Unknown" ) ) ) );


	// Special Weapons
	if(Weap->Type == WPN_SPECIAL) {
		
		fprintf(fp,"Special = %s\n", (
					Weap->Special == SPC_JETPACK ? "SPC_JETPACK" : "Unknown" ) );

		// If it is a special weapon, read the values depending on the special weapon
		// We don't bother with the 'normal' values
		switch(Weap->Special) {
			// Jetpack
			case SPC_JETPACK:
				DecompileJetpack(fp, Weap);
				break;

			default:
				printf("   Error: Unknown special type\n");
		}
		return true;
	}


	// Beam Weapons
	if(Weap->Type == WPN_BEAM) {

		DecompileBeam(fp,Weap);
		return true;
	}


	// Projectile Weapons
	fprintf( fp, "Recoil = %i\n",Weap->Recoil);
	fprintf( fp, "Drain = %f\n", Weap->Drain);
	fprintf( fp, "Recharge = %f\n", Weap->Recharge);
	fprintf( fp, "ROF = %f\n",Weap->ROF);
	if( Weap->UseSound )
		fprintf( fp, "Sound = %s\n",Weap->SndFilename);
	fprintf( fp, "LaserSight = %s\n", (Weap->LaserSight ? "true":"false"));
	fprintf( fp, "Class = %s\n", (
				Weap->Class == WCL_AUTOMATIC ? "WCL_AUTOMATIC" : (
				Weap->Class == WCL_POWERGUN ? "WCL_POWERGUN" : (
				Weap->Class == WCL_GRENADE ? "WCL_GRENADE" : (
				Weap->Class == WCL_CLOSERANGE ? "WCL_CLOSERANGE" : "Unknown" )))));

	fprintf( fp,"\n[Projectile]\n\nSpeed = %i\n",Weap->ProjSpeed);
	fprintf( fp, "SpeedVar = %f\n", Weap->ProjSpeedVar);
	fprintf( fp, "Spread = %f\n", Weap->ProjSpread);
	fprintf( fp, "Amount = %i\n", Weap->ProjAmount);
	fprintf( fp, "Projectile = p_%s_%08x.txt\n", Weap->Name, (int)Weap->Projectile);
	
	fclose(fp);
	
	DecompileProjectile( Weap->Projectile, Weap->Name );

	return true;
}


///////////////////
// Compile a beam weapon
void DecompileBeam(FILE * fp, weapon_t *Weap)
{
	fprintf( fp,"\n[General]\n\nRecoil = %i\n",Weap->Recoil);
	fprintf( fp, "Drain = %f\n", Weap->Drain);
	fprintf( fp, "Recharge = %f\n", Weap->Recharge);
	fprintf( fp, "ROF = %f\n",Weap->ROF);
	if( Weap->UseSound )
		fprintf( fp, "Sound = %s\n",Weap->SndFilename);

	fprintf( fp, "\n[Beam]\n\nDamage = %i\n", Weap->Bm_Damage);
	fprintf( fp, "Length = %i\n", Weap->Bm_Length);
	fprintf( fp, "PlayerDamage = %i\n", Weap->Bm_PlyDamage);
	fprintf( fp, "Colour = %u,%u,%u\n", Weap->Bm_Colour[0], Weap->Bm_Colour[1], Weap->Bm_Colour[2]);

}


///////////////////
// Compile a projectile
void DecompileProjectile(proj_t * proj, const char * weaponName)
{
	char fname[128];

	sprintf(fname, "p_%s_%08x.txt", weaponName, (int)proj);
	
	FILE * fp = fopen(fname, "w");
	if(!fp)
		return;

	ProjCount++;

	printf("    Compiling Projectile '%s'\n",fname);

	fprintf(fp,"[General]\n\nType = %s\n", (
				proj->Type == PRJ_PIXEL ? "PRJ_PIXEL" : (
				proj->Type == PRJ_IMAGE ? "PRJ_IMAGE" : "Unknown" )));

	fprintf( fp, "Timer = %f\n", proj->Timer_Time);
	fprintf( fp, "TimerVar = %f\n", proj->Timer_TimeVar);
	fprintf( fp, "Trail = %s\n", (
				proj->Trail == TRL_NONE ? "TRL_NONE" : ( 
				proj->Trail == TRL_SMOKE ? "TRL_SMOKE" : ( 
				proj->Trail == TRL_CHEMSMOKE ? "TRL_CHEMSMOKE" : ( 
				proj->Trail == TRL_PROJECTILE ? "TRL_PROJECTILE" : ( 
				proj->Trail == TRL_DOOMSDAY ? "TRL_DOOMSDAY" : ( 
				proj->Trail == TRL_EXPLOSIVE ? "TRL_EXPLOSIVE" : " Unknown" 
				)))))));
	

	if( proj->UseCustomGravity )
		fprintf( fp, "Gravity = %i\n", proj->Gravity);
	
	fprintf( fp, "Dampening = %f\n", proj->Dampening);

	if(proj->Type == PRJ_PIXEL) {

		fprintf( fp, "Colour1 = %u,%u,%u\n", proj->Colour1[0], proj->Colour1[1], proj->Colour1[2]);

		if( proj->NumColours >= 2 )
			fprintf( fp, "Colour2 = %u,%u,%u\n", proj->Colour2[0], proj->Colour2[1], proj->Colour2[2]);

	} else if(proj->Type == PRJ_IMAGE) {
		fprintf( fp, "Image = %s\n", proj->ImgFilename);
		fprintf( fp, "Rotating = %s\n", ( proj->Rotating ? "true":"false" ));
		fprintf( fp, "RotIncrement = %i\n", proj->RotIncrement);
		fprintf( fp, "RotSpeed = %i\n", proj->RotSpeed);
		fprintf( fp, "UseAngle = %s\n", ( proj->UseAngle ? "true":"false" ));
		fprintf( fp, "UseSpecAngle = %s\n", ( proj->UseSpecAngle ? "true":"false" ));
		if(proj->UseAngle || proj->UseSpecAngle)
			fprintf( fp, "AngleImages = %i\n", proj->AngleImages);
	
		fprintf( fp, "Animating = %s\n", ( proj->Animating ? "true":"false" ));

		if(proj->Animating) {
			fprintf( fp, "AnimRate = %f\n", proj->AnimRate);
			fprintf( fp, "AnimType = %s\n", (
						proj->AnimType == ANI_ONCE ? "ANI_ONCE" : (
						proj->AnimType == ANI_LOOP ? "ANI_LOOP" : (
						proj->AnimType == ANI_PINGPONG ? "ANI_PINGPONG" : "Unknown" ))));
		}
	}
	

	fprintf( fp, "\n[Hit]\n" );
	fprintf( fp, "Type = %s\n", (
				proj->Hit_Type == PJ_BOUNCE ? "Bounce" : (
				proj->Hit_Type == PJ_EXPLODE ? "Explode" : (
				proj->Hit_Type == PJ_INJURE ? "Injure" : (
				proj->Hit_Type == PJ_CARVE ? "Carve" : (
				proj->Hit_Type == PJ_DIRT ? "Dirt" : (
				proj->Hit_Type == PJ_GREENDIRT ? "GreenDirt" : (
				proj->Hit_Type == PJ_DISAPPEAR ? "Disappear" : (
				proj->Hit_Type == PJ_NOTHING ? "Nothing" : "Unknown" )))))))));


	// Hit::Explode
	if(proj->Hit_Type == PJ_EXPLODE) {

		fprintf( fp, "Damage = %i\n", proj->Hit_Damage);
		fprintf( fp, "Projectiles = %s\n", ( proj->Hit_Projectiles ? "true":"false" ));
		fprintf( fp, "Shake = %i\n", proj->Hit_Shake);
		
		if( proj->Hit_UseSound )
			fprintf( fp, "Sound = %s\n", proj->Hit_SndFilename);
	}

	// Hit::Carve
	if(proj->Hit_Type == PJ_CARVE) {
		fprintf( fp, "Damage = %i\n", proj->Hit_Damage);
	}

	// Hit::Bounce
	if(proj->Hit_Type == PJ_BOUNCE) {
		fprintf( fp, "BounceCoeff = %f\n", proj->Hit_BounceCoeff);
		fprintf( fp, "BounceExplode = %i\n", proj->Hit_BounceExplode);
	}

	// Timer
	if(proj->Timer_Time > 0) {
	
		fprintf( fp, "\n[Time]\n" );
		fprintf( fp, "Type = %s\n", (
				proj->Timer_Type == PJ_BOUNCE ? "Bounce" : (
				proj->Timer_Type == PJ_EXPLODE ? "Explode" : (
				proj->Timer_Type == PJ_INJURE ? "Injure" : (
				proj->Timer_Type == PJ_CARVE ? "Carve" : (
				proj->Timer_Type == PJ_DIRT ? "Dirt" : (
				proj->Timer_Type == PJ_GREENDIRT ? "GreenDirt" : (
				proj->Timer_Type == PJ_DISAPPEAR ? "Disappear" : (
				proj->Timer_Type == PJ_NOTHING ? "Nothing" : "Unknown" )))))))));
		
		if(proj->Timer_Type == PJ_EXPLODE) {

			fprintf( fp, "Damage = %i\n", proj->Timer_Damage);
			fprintf( fp, "Projectiles = %s\n", ( proj->Timer_Projectiles ? "true":"false" ));
			fprintf( fp, "Shake = %i\n", proj->Timer_Shake);
		}
	}

	// Player hit
	fprintf( fp, "\n[PlayerHit]\n" );
	fprintf( fp, "Type = %s\n", (
				proj->PlyHit_Type == PJ_BOUNCE ? "Bounce" : (
				proj->PlyHit_Type == PJ_EXPLODE ? "Explode" : (
				proj->PlyHit_Type == PJ_INJURE ? "Injure" : (
				proj->PlyHit_Type == PJ_CARVE ? "Carve" : (
				proj->PlyHit_Type == PJ_DIRT ? "Dirt" : (
				proj->PlyHit_Type == PJ_GREENDIRT ? "GreenDirt" : (
				proj->PlyHit_Type == PJ_DISAPPEAR ? "Disappear" : (
				proj->PlyHit_Type == PJ_NOTHING ? "Nothing" : "Unknown" )))))))));


	// PlyHit::Explode || PlyHit::Injure
	if(proj->PlyHit_Type == PJ_EXPLODE || proj->PlyHit_Type == PJ_INJURE) {
		fprintf( fp, "Damage = %i\n", proj->PlyHit_Damage);
		fprintf( fp, "Projectiles = %s\n", ( proj->PlyHit_Projectiles ? "true":"false" ));
	}

	// PlyHit::Bounce
	if(proj->PlyHit_Type == PJ_BOUNCE) {
		fprintf( fp, "BounceCoeff = %f\n", proj->PlyHit_BounceCoeff);
	}


    // OnExplode
	fprintf( fp, "\n[Explode]\n" );
	fprintf( fp, "Type = %s\n", (
				proj->Exp_Type == PJ_BOUNCE ? "Bounce" : (
				proj->Exp_Type == PJ_EXPLODE ? "Explode" : (
				proj->Exp_Type == PJ_INJURE ? "Injure" : (
				proj->Exp_Type == PJ_CARVE ? "Carve" : (
				proj->Exp_Type == PJ_DIRT ? "Dirt" : (
				proj->Exp_Type == PJ_GREENDIRT ? "GreenDirt" : (
				proj->Exp_Type == PJ_DISAPPEAR ? "Disappear" : (
				proj->Exp_Type == PJ_NOTHING ? "Nothing" : "Unknown" )))))))));

	fprintf( fp, "Damage = %i\n", proj->Exp_Damage);
	fprintf( fp, "Projectiles = %s\n", ( proj->Exp_Projectiles ? "true":"false" ));
	fprintf( fp, "Shake = %i\n", proj->Exp_Shake);
	if( proj->Exp_UseSound )
		fprintf( fp, "Sound = %s\n", proj->Exp_SndFilename);


    // Touch
	fprintf( fp, "\n[Touch]\n" );
	fprintf( fp, "Type = %s\n", (
				proj->Tch_Type == PJ_BOUNCE ? "Bounce" : (
				proj->Tch_Type == PJ_EXPLODE ? "Explode" : (
				proj->Tch_Type == PJ_INJURE ? "Injure" : (
				proj->Tch_Type == PJ_CARVE ? "Carve" : (
				proj->Tch_Type == PJ_DIRT ? "Dirt" : (
				proj->Tch_Type == PJ_GREENDIRT ? "GreenDirt" : (
				proj->Tch_Type == PJ_DISAPPEAR ? "Disappear" : (
				proj->Tch_Type == PJ_NOTHING ? "Nothing" : "Unknown" )))))))));

	fprintf( fp, "Damage = %i\n", proj->Tch_Damage);
	fprintf( fp, "Projectiles = %s\n", ( proj->Tch_Projectiles ? "true":"false" ));
	fprintf( fp, "Shake = %i\n", proj->Tch_Shake);
	if( proj->Exp_UseSound )
		fprintf( fp, "Sound = %s\n", proj->Tch_SndFilename);


	// Projectiles
	if(proj->Timer_Projectiles || proj->Hit_Projectiles || proj->PlyHit_Projectiles || proj->Exp_Projectiles ||
       proj->Tch_Projectiles) {
	
		fprintf( fp, "\n[Projectile]\n" );
		fprintf( fp, "Projectile = p_%s_%08x.txt\n", weaponName, (int)proj->Projectile);
		fprintf( fp, "Amount = %i\n", proj->ProjAmount);
		fprintf( fp, "Speed = %i\n", proj->ProjSpeed);
		fprintf( fp, "SpeedVar = %f\n", proj->ProjSpeedVar);
		fprintf( fp, "Spread = %f\n", proj->ProjSpread);
		fprintf( fp, "Useangle = %s\n", ( proj->ProjUseangle ? "true":"false" ));
		fprintf( fp, "Angle = %i\n", proj->ProjAngle);

		DecompileProjectile(proj->Projectile, weaponName);
	}


	// Projectile trail
	if(proj->Trail == TRL_PROJECTILE) {

		fprintf( fp, "\n[ProjectileTrail]\n" );
		fprintf( fp, "Projectile = p_%s_%08x.txt\n", weaponName, (int)proj->PrjTrl_Proj);
		fprintf( fp, "Delay = %f\n", proj->PrjTrl_Delay);
		fprintf( fp, "Amount = %i\n", proj->PrjTrl_Amount);
		fprintf( fp, "Speed = %i\n", proj->PrjTrl_Speed);
		fprintf( fp, "SpeedVar = %f\n", proj->PrjTrl_SpeedVar);
		fprintf( fp, "Spread = %f\n", proj->PrjTrl_Spread);

		DecompileProjectile(proj->PrjTrl_Proj, weaponName);
	}
}


///////////////////
// Compile the extra stuff
int DecompileExtra(FILE * fp)
{
	char file[64];

	int ropel, restl;
	float strength;

	fprintf(fp,"\n[NinjaRope]\n\nRopeLength = %i\n", Game->getRopeLength());
	fprintf(fp, "RestLength = %i\n", Game->getRestLength() );
	fprintf(fp, "Strength = %f\n",Game->getStrength());

	gs_worm_t *wrm = Game->getWorm();
	
	fprintf(fp,"\n[Worm]\n\nAngleSpeed = %f\n", wrm->AngleSpeed );
	fprintf(fp, "GroundSpeed = %f\n", wrm->GroundSpeed );
	fprintf(fp, "AirSpeed = %f\n", wrm->AirSpeed );
	fprintf(fp, "Gravity = %f\n", wrm->Gravity );
	fprintf(fp, "JumpForce = %f\n", wrm->JumpForce );
	fprintf(fp, "AirFriction = %f\n", wrm->AirFriction );
	fprintf(fp, "GroundFriction = %f\n", wrm->GroundFriction );

	return true;
}


/*
===============================

		Special items

===============================
*/


///////////////////
// Compile the jetpack
int DecompileJetpack(FILE * fp, weapon_t *Weap)
{

	fprintf( fp, "[JetPack]\n\nThrust = %i\n", Weap->tSpecial.Thrust);
	fprintf( fp, "Drain = %f\n", Weap->Drain);
	fprintf( fp, "Recharge = %f\n", Weap->Recharge);

	return true;
}
