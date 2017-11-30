//
//  AALA.c
//  iPoS_IOS
//
//  Created by mr.ma on 29/08/2017.
//  Copyright © 2017 Treasure Frontier System Sdn. Bhd. All rights reserved.
//

#include "AALA.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ArrayUtils.h"
#include "SharedVariables.h"
#include "QuotationDB.h"
#include "InputConstant.h"
#include "MathUtils.h"
#include "FundBase.h"

#include "HI3.h"
#include "HSP3.h"
#include "CI3.h"
#include "PA3.h"
#include "PW3.h"
#include "StructBean.h"
#include "PT3.h"
#include "SW4.h"
#include "WOP4.h"

#include "PHS1.h"
#include "PHS_PLUS_AIPA_USD.h"
#include "C3_AIPA.h"
#include "SW_C3_AIPA.h"
#include "WOP_C3_AIPA.h"
#include "PHS_PLUS_AIPA_IDR.h"
#include "PA_AIPA.h"

double aala_exchangeRate;
int aala_illustrationValid;
int aala_allowedWdrlPremium;

// Interfaces

int aala_checkWaiver(void);

int aala_illustrationChecking(double **data, struct basic policy,
                              int withdrawalIndex, double *maxWithdrawalValue, int cashValueIndex,
                              int ageIndex);

void aala_updateWaiver(int driven, double value);
void aala_calculateFunds(double **data, int scenario);

double aala_getFreqPremium(int minMax);
double aala_getSA(int minMax);
double aala_getPremium(int driven, int minMax);
double aala_getExchangeRate(const char *prefix);
//double aala_getPolicyCharge(const char *prefix);
double aala_checkATP_rate(double *atp, struct person person);
double aala_getCOI(struct basic policy, int gender, int smoker, int age);
double aala_getBasicPremium(int durMonth, int aph, double premium);
double aala_getPlanCoi(const char *prefix, int gender, int smoker, int age);
double aala_getCor(int year, double corCurrent, struct rider rider,
                   int coiWaiverMonth);
double aala_getCorRate(int year, double cor, struct rider rider,
                       int coiWaiverMonth);
double aala_updateWaiverSA(struct rider rider, int driven, double value,
                           struct basic policy);
double aala_getRP_ADI(int year, double value, int checkValidasi,
                      double RP_END_TOTAL, double TP_END_TOTAL);
double aala_getWaiverCost(int year, double sar, double cor, struct rider rider,
                          struct basic policy, int coiWaiverMonth);
double aala_getManfaatRisikoMeninggal(int year, double sumInsured, double tebus,
                                      double rpTotal, double tpTotal, double lien);
double aala_getDeathBenefit(int isDeathBenefitValid, double deathBenefit,
                            double sumAssured, double rpTotal, double tpTotal, double lien,
                            int multiplier);
double aala_getRateCostInsurance(int durMonth, double sumRisk,
                                 struct basic policy, int insuredAge, double validasi, double *atp,
                                 double coiDiscount);
double aala_getWaiverAtp_rate(struct rider rider, double atp, double sar);

double *aala_getATP_rate(void);
double *aala_getATPSubStandard(void);
double *aala_getTotalCor(int *size);
double *aala_getLien(const char *prefix, int *size);
double *aala_getAphCharge(const char *prefix, int *size);
double *aala_getCoiDiscount(const char *prefix, int *size);
double *aala_getTopupAllocation(const char *prefix, int *size);
double *aala_getSurrenderCharge(const char *prefix, int *size);
double *aala_getMonthlyPolicyFee(const char *prefix, int *size);
double *aala_getFoundationCharge(const char *prefix, int *size);
double *aala_getPremiumAllocation(const char *prefix, int *size);

double **aala_calculateBasic(int scenario, int *row, int *col);

//different methods with affp

double *aala_getAdb(const char *prefix, int *size);
double *aala_getIndemnity(const char *prefix, int *size);
double *aala_getPABenefit(const char *prefix, int *size);
double *aala_getPctgLoyaltyBonus(const char *prefix, int *size);
double *aala_getPctgPotensiLoyaltyBonus(const char *prefix, int *size);

int aala_getNoLapseGuaranteeMonth(const char *prefix);
int aala_getCoiWaiverMonth(const char *prefix);
int aala_getMaxLoyaltyBonusYear(const char *prefix);

double aala_getAphCharges(int durMonth, double aphCharges);
//double aala_getMaxSAMultiplier(const char *prefix, int age);

// Implementation
/* DB
 * */
double *aala_getCoi(const char *prefix, int gender, int smoker, int *size)
{
    if (rateDb == NULL)
    {
        return NULL;
    }
    
    char sql[100];
    sprintf(sql, "SELECT %s%s FROM %s_COI", gender == FEMALE ? "F" : "M",
            smoker == SMOKER ? "S" : "NS", prefix);
    return getDoubleColumnFromDb(rateDb, sql, size);
}

double aala_getPlanCoi(const char *prefix, int gender, int smoker, int age)
{
    if (rateDb == NULL)
    {
        return 0;
    }
    
    char sql[100];
    sprintf(sql, "SELECT %s%s FROM %s_COI WHERE age = %d",
            gender == FEMALE ? "F" : "M", smoker == SMOKER ? "S" : "NS", prefix,
            age);
    return getDoubleValueFromDb(rateDb, sql);
}

double *aala_getAtp(const char *prefix, const char *variant, int gender,
                    int smoker, int *size)
{
    if (rateDb == NULL)
    {
        return NULL;
    }
    char sql[100];
    const char *payterm ;
    //    if (strcmp(policy.code, "UXR")==0||strcmp(policy.code, "UXD")==0) {
    payterm ="W";
    //    }else{
    //        payterm ="T";
    //    }
    sprintf(sql, "SELECT %s%s_%s_%s FROM %s_ATP", gender == FEMALE ? "F" : "M",
            smoker == SMOKER ? "S" : "NS", variant, payterm,prefix);
    return getDoubleColumnFromDb(rateDb, sql, size);
}

double *aala_getPremiumAllocation(const char *prefix, int *size)
{
    if (rateDb == NULL)
    {
        return NULL;
    }
    
    char sql[100];
    sprintf(sql, "SELECT value FROM %s_premium_allocation", prefix);
    return getDoubleColumnFromDb(rateDb, sql, size);
}

double *aala_getTopupAllocation(const char *prefix, int *size)
{
    if (rateDb == NULL)
    {
        return NULL;
    }
    
    char sql[100];
    sprintf(sql, "SELECT value FROM %s_topup_allocation", prefix);
    return getDoubleColumnFromDb(rateDb, sql, size);
}

double *aala_getSurrenderCharge(const char *prefix, int *size)
{
    if (rateDb == NULL)
    {
        return NULL;
    }
    
    char sql[100];
    sprintf(sql, "SELECT value FROM %s_surrender_charge", prefix);
    return getDoubleColumnFromDb(rateDb, sql, size);
}

double *aala_getMonthlyPolicyFee(const char *prefix, int *size)
{
    if (rateDb == NULL)
    {
        return NULL;
    }
    
    char sql[100];
    sprintf(sql, "SELECT value FROM %s_monthly_policy_fee", prefix);
    return getDoubleColumnFromDb(rateDb, sql, size);
}

double *aala_getAphCharge(const char *prefix, int *size)
{
    if (rateDb == NULL)
    {
        return NULL;
    }
    
    char sql[100];
    sprintf(sql, "SELECT value FROM %s_aph_charge", prefix);
    return getDoubleColumnFromDb(rateDb, sql, size);
}

double *aala_getLien(const char *prefix, int *size)
{
    if (rateDb == NULL)
    {
        return NULL;
    }
    
    char sql[100];
    sprintf(sql, "SELECT value FROM %s_lien", prefix);
    return getDoubleColumnFromDb(rateDb, sql, size);
}

double *aala_getAdb(const char *prefix, int *size)
{
    if (rateDb == NULL)
    {
        return NULL;
    }
    
    char sql[100];
    sprintf(sql, "SELECT value FROM %s_adb", prefix);
    return getDoubleColumnFromDb(rateDb, sql, size);
}

double *aala_getIndemnity(const char *prefix, int *size)
{
    if (rateDb == NULL)
    {
        return NULL;
    }
    
    char sql[100];
    sprintf(sql, "SELECT value FROM %s_indemnity", prefix);
    return getDoubleColumnFromDb(rateDb, sql, size);
}

double *aala_getPABenefit(const char *prefix, int *size)
{
    if (rateDb == NULL)
    {
        return NULL;
    }
    
    char sql[100];
    sprintf(sql, "SELECT value FROM %s_pa_benefit", prefix);
    return getDoubleColumnFromDb(rateDb, sql, size);
}

double *aala_getCoiDiscount(const char *prefix, int *size)
{
    if (rateDb == NULL)
    {
        return NULL;
    }
    
    char sql[100];
    sprintf(sql, "SELECT value FROM %s_coi_discount", prefix);
    return getDoubleColumnFromDb(rateDb, sql, size);
}

double *aala_getFoundationCharge(const char *prefix, int *size)
{
    if (rateDb == NULL)
    {
        return NULL;
    }
    
    char sql[100];
    sprintf(sql, "SELECT value FROM %s_foundation_charge", prefix);
    return getDoubleColumnFromDb(rateDb, sql, size);
}

double aala_getExchangeRate(const char *prefix)
{
    if (rateDb == NULL)
    {
        return 0;
    }
    
    char sql[100];
    sprintf(sql, "SELECT exchangeRate FROM %s_misc", prefix);
    return getDoubleValueFromDb(rateDb, sql);
}

//FEUL++ policy charge is different based on age
//double aala_getPolicyCharge(const char *prefix)
//{
//    if (rateDb == NULL)
//    {
//        return 0;
//    }
//
//    char sql[100];
//    sprintf(sql, "SELECT policyCharge FROM %s_misc", prefix);
//    return getDoubleValueFromDb(rateDb, sql);
//}

int aala_getMaxLoyaltyBonusYear(const char *prefix)
{
    if (rateDb == NULL)
    {
        return 0;
    }
    
    char sql[100];
    sprintf(sql, "SELECT maxLoyaltyBonusYear FROM %s_misc", prefix);
    return getIntegerValueFromDb(rateDb, sql);
}

double *aala_getPctgLoyaltyBonus(const char *prefix, int *size)
{
    if (rateDb == NULL)
    {
        return NULL;
    }
    
    char sql[100];
    sprintf(sql, "SELECT value FROM %s_loyalty_bonus", prefix);
    return getDoubleColumnFromDb(rateDb, sql, size);
}
double *aala_getPctgPotensiLoyaltyBonus(const char *prefix, int *size)
{
    if (rateDb == NULL)
    {
        return NULL;
    }
    
    char sql[100];
    sprintf(sql, "SELECT value FROM %s_potential_loyalty_bonus", prefix);
    return getDoubleColumnFromDb(rateDb, sql, size);
}

double aala_getMinSAMultiplier(const char *prefix, int age)
{
    if (rateDb == NULL)
    {
        return 0;
    }
    
    char sql[100];
    sprintf(sql, "SELECT minimum FROM %s_sa_multiplier WHERE age = %d", prefix,
            age);
    return getDoubleValueFromDb(rateDb, sql);
}

//double aala_getMaxSAMultiplier(const char *prefix, int age)
//{
//    if (rateDb == NULL)
//    {
//        return 0;
//    }
//
//    char sql[100];
//    sprintf(sql, "SELECT maximum FROM %s_sa_multiplier WHERE age = %d", prefix,
//    age);
//    return getDoubleValueFromDb(rateDb, sql);
//}
/* XXX ---------------------------------------- Calculation ---------------------------------------- */

/*
 * calculate premium
 */
double aala_getBasicPremium(int durMonth, int aph, double premium)
{
    if (aph == 1)
    {
        return 0;
    }
    
    if ((durMonth / 12.0) > policy.premiumTerm)
    {
        return 0;
    }
    else
    {
        if ((durMonth % 12 == 1 && policy.freq == 1)
            || (durMonth % 6 == 1 && policy.freq == 2)
            || (durMonth % 3 == 1 && policy.freq == 4)
            || (durMonth % 1 == 0 && policy.freq == 12))
        {
            return premium;
        }
        else
        {
            return 0;
        }
    }
}

/*
 * get cost of insurance
 */
double aala_getRateCostInsurance(int durMonth, double sumRisk,
                                 struct basic policy, int insuredAge, double validasi, double *atp,
                                 double coiDiscount)
{
    double percentLoading = roundDoubleDown(
                                            1 + policy.medPercentLoading + policy.occPercentLoading, -6);
//    double perMilLoading = roundDoubleDown(
//                                           (policy.medPerMilLoading + policy.occPerMilLoading) / 12, -6)
//    + policy.gePerMilLoading;
    
    double perMilLoading = (policy.medPerMilLoading + policy.occPerMilLoading) / 12
    + policy.gePerMilLoading;

    double value1 = atp[insuredAge - 1] * percentLoading + perMilLoading;
    
    double value2 = 1 - coiDiscount;
    return value1 * value2;
}

/*
 * get total manfaat risiko meninggal
 */
double aala_getManfaatRisikoMeninggal(int year, double sumInsured, double tebus,
                                      double rpTotal, double tpTotal, double lien)
{
    double value = sumInsured * lien;
    
    if (tebus <= 0 && year > policy.noLapseGuaranteeYear)
    {
        return 0;
    }
    else
    {
        return value + rpTotal + tpTotal;
    }
}

/*
 * get total manfaat risiko meninggal ADB & Indemnity
 */
double aala_getDeathBenefit(int isDeathBenefitValid, double deathBenefit,
                            double sumAssured, double rpTotal, double tpTotal, double lien,
                            int multiplier)
{
    if (isDeathBenefitValid != 0)
    {
        return 0;
    }
    else
    {
        if (deathBenefit == 0)
        {
            return sumAssured + rpTotal + tpTotal;
        }
        else
        {
            return multiplier * minDouble(sumAssured, deathBenefit)
            + sumAssured * lien + rpTotal + tpTotal;
        }
    }
}

/*
 * calculate Alokasi Dana Investasi (Regular Premium)
 */
double aala_getRP_ADI(int year, double value, int checkValidasi,
                      double RP_END_TOTAL, double TP_END_TOTAL)
{
    if (year > policy.noLapseGuaranteeYear || checkValidasi)
    {
        if (RP_END_TOTAL <= 0 && TP_END_TOTAL <= 0)
        {
            return 0;
        }
        else
        {
            return value < 0 ? 0 : value;
        }
    }
    
    return value;
}

/*
 * calculate COR rate
 */
double aala_getCorRate(int year, double cor, struct rider rider,
                       int coiWaiverMonth)
{
    int isCoiWaived = (year <= coiWaiverMonth / 12);
    
    if (!hasCor(rider, year) || isCoiWaived)
    {
        return 0;
    }
    
    double medPercentLoading = roundDoubleDown(
                                               1 + rider.medPercentLoading + rider.occPercentLoading, -6);
    double perMilLoading = roundDoubleDown(
                                           (rider.medPerMilLoading + rider.occPerMilLoading) / 12, -6)
    + rider.gePerMilLoading;
    
    return cor * medPercentLoading + perMilLoading;
}

/*
 * calculate COR
 */
double aala_getCor(int year, double corCurrent, struct rider rider,
                   int coiWaiverMonth)
{
    if (!hasCor(rider, year))
    {
        return 0;
    }
    
    double corRate = aala_getCorRate(year, corCurrent, rider, coiWaiverMonth);
    
    //    double cor = corRate * (rider.sumAssured / 1000);
    if ((corRate * (rider.sumAssured / 1000) - roundDouble(corRate * (rider.sumAssured / 1000), -2))<-0.005)
    {
        return roundDoubleDown(corRate * (rider.sumAssured / 1000), -2);
    }
    else
    {
        return roundDouble(corRate * (rider.sumAssured / 1000), -2);
    }
    
    //	return roundPrecisionHalfToEven(corRate * (rider.sumAssured / 1000), 0);
    //    return roundDouble(cor, -2);
}

/*
 * calculate waiver rider cost
 */
double aala_getWaiverCost(int year, double sar, double cor, struct rider rider,
                          struct basic policy, int coiWaiverMonth)
{
    int isCoiWaived = (year <= coiWaiverMonth / 12);
    
    if (!hasCor(rider, year) || isCoiWaived)
    {
        return 0;
    }
    else
    {
        double premiumSa = policy.freq * policy.premium;
        double topUpSa = policy.freq * policy.regularTopUp;
        
        if (rider.sumAssured > policy.annualPremium)
        {
            premiumSa += fmin(topUpSa, (2 * premiumSa));
        }
        
        double medPercentLoading = roundDoubleDown(
                                                   1 + rider.medPercentLoading + rider.occPercentLoading, -6);
        double perMilLoading = roundDoubleDown(
                                               (rider.medPerMilLoading + rider.occPerMilLoading) / 12, -6)
        + rider.gePerMilLoading;
        
        return roundDouble(
                           (cor * medPercentLoading + perMilLoading)
                           * (premiumSa * sar / 1000), -2);
    }
}

/*
 get COI
 */
double aala_getCOI(struct basic policy, int gender, int smoker, int age)
{
    double rate = aala_getPlanCoi(policy.prefix, gender, smoker, age);
    double medLoading = roundDoubleDown(
                                        1 + policy.medPercentLoading + policy.occPercentLoading, -12);
    double perMilLoading = roundDoubleDown(
                                           (policy.medPerMilLoading + policy.occPerMilLoading) / 12, -12)
    + policy.gePerMilLoading;
    
    //    return (rate * medLoading + perMilLoading) * policy.sumAssured / 1000;
    // TT1314 add by Knight
    return roundDouble((rate * medLoading + perMilLoading) * policy.sumAssured / 1000, -2);
}

/*
 * Different methods wtih affp
 * */
double aala_getAphCharges(int durMonth, double aphCharges)
{
    if ((durMonth / 12.0) > policy.premiumTerm)
    {
        return 0;
    }
    else
    {
        if ((durMonth % 12 == 1 && policy.freq == 1)
            || (durMonth % 6 == 1 && policy.freq == 2)
            || (durMonth % 3 == 1 && policy.freq == 4)
            || (durMonth % 1 == 0 && policy.freq == 12))
        {
            return aphCharges;
        }
        else
        {
            return 0;
        }
    }
}

int aala_getNoLapseGuaranteeMonth(const char *prefix)
{
    if (rateDb == NULL)
    {
        return 0;
    }
    
    char sql[100];
    sprintf(sql, "SELECT noLapseGuaranteeMonth FROM %s_misc", prefix);
    
    return getIntegerValueFromDb(rateDb, sql);
}

// Get the month before COI or COR waived
int aala_getCoiWaiverMonth(const char *prefix)
{
    if (rateDb == NULL)
    {
        return 0;
    }
    
    char sql[100];
    sprintf(sql, "SELECT coiWaiverMonth FROM %s_misc", prefix);
    
    return getIntegerValueFromDb(rateDb, sql);
}

void aala_calculateFunds(double **data, int scenario)
{
    int checkWithdrawal = 0;
    double APLAccTotalCharges = 0;
    double APLWDRLTotal = 0;
    double maxLoyaltyBonus = 0;
    double totalRegularPremiumWdwl = 0;
    double sumTotalRPWdrl = 0;
    
    int maxLoyaltyBonusYear = aala_getMaxLoyaltyBonusYear(policy.prefix);
    
    double ***funds = make3DDoubleArray(policy.coverageTerm * 12,
                                        policy.fundSize, AALA_FUND_CAL_SIZE);
    double *lien = aala_getLien(policy.prefix, NULL);
    double *adb = aala_getAdb(policy.prefix, NULL);
    double *indemnity = aala_getIndemnity(policy.prefix, NULL);
    //double sumAssured = policy.sumAssured;
    double *coiDiscount = aala_getCoiDiscount(policy.prefix, NULL);
    double *coi = aala_getCoi(policy.prefix, insured.sex, insured.smoker, NULL);
    //double *totalCor = aala_getTotalCor(NULL);
    double *foundationCharge = aala_getFoundationCharge(policy.prefix, NULL);
    double *pctgLoyaltyBonus = aala_getPctgLoyaltyBonus(policy.prefix, NULL);
    double *pctgPotensiLoyaltyBonus = aala_getPctgPotensiLoyaltyBonus(
                                                                      policy.prefix, NULL);
    double tempadjust =0;
    
    //  double loyaltyBonusSum = 0;
    //  double extraAllocationSum = 0;
    int i, j;
    
    for (i = 0; i < policy.coverageTerm; i++)
    {
        maxLoyaltyBonus += pctgLoyaltyBonus[i];
    }
    
    for (i = 0; i < policy.coverageTerm; i++)
    {
        const int year = i + 1;
        const int insuredAge = insured.age + i;
        
        for (j = 0; j < 12; j++)
        {
            
            const int index = i * 12 + j;
            
            //			const int month = j + 1;
            const int durMonth = index + 1;
            
            int noLapseGuaranteeMonth = aala_getNoLapseGuaranteeMonth(
                                                                      policy.prefix);
            
            /*
             *Before fund calculation
             */
            
            data[index][AALA_PCTG_FOUNDATION_CHARGE] = (data[index][AALA_RP_PCTG_ACQUISITION_COST] == 1 && durMonth <= 12) ? 0 : foundationCharge[year - 1] / 12;
            
            //NO LAPSE GUARANTEED INDICATOR
            
            data[index][AALA_NLGI_WITHDRAWAL] = (durMonth <= 1) ? 0 : (data[index - 1][AALA_NLGI_WITHDRAWAL] == 1 ? 1 : (checkWithdrawal ? 1 : 0));
            
            data[index][AALA_NLGI_VALIDASI] = (data[index][AALA_NLGI_APH] == 1|| data[index][AALA_NLGI_WITHDRAWAL] == 1) ? 1 : 0;
            
            data[index][AALA_RATE_COST_OF_INSURANCE] = aala_getRateCostInsurance(
                                                                                 durMonth,
                                                                                 (index > 0 ? data[index - 1][AALA_SUM_AT_RISK] : 0), policy,
                                                                                 insuredAge, data[index][AALA_NLGI_VALIDASI], coi,
                                                                                 coiDiscount[year - 1]);
            
            double aphCharges = policy.premium * data[index][AALA_PCTG_APH_CHARGES];
            
            data[index][AALA_APH_CHARGES] = data[index][AALA_APH] > 0 ? aala_getAphCharges(durMonth, aphCharges) : 0;
            
            double lastDeathBenefit =
            index > 0 ? data[index - 1][AALA_TOTAL_MANFAAT_RISIKO_MENINGGAL] : 1;
            
            data[index][AALA_TP_RP] =
            lastDeathBenefit <= 0 || isnan(lastDeathBenefit) ? 0 : aala_getBasicPremium(durMonth,
                                                                                        (int) data[index][AALA_APH],
                                                                                        policy.regularTopUp);
            
            data[index][AALA_TP_TOTAL_PREMIUM] = data[index][AALA_TP_RP]
            + data[index][AALA_TP_AD_HOC_PREMIUM];
            
            if (year <= policy.coverageTerm && year > 0) {
                data[index][AALA_TP_ACCUMULATED_TOTAL_PREMIUM] =(index > 0 ?data[index][AALA_TP_TOTAL_PREMIUM]
                                                                 + data[index - 1][AALA_TP_ACCUMULATED_TOTAL_PREMIUM] : data[index][AALA_TP_TOTAL_PREMIUM]);
            }else
                data[index][AALA_TP_ACCUMULATED_TOTAL_PREMIUM] = 0;
            //            data[index][AALA_TP_ACCUMULATED_TOTAL_PREMIUM] =
            //            year <= policy.coverageTerm && year > 0 ?
            //            index > 0 ?
            //            data[index][AALA_TP_TOTAL_PREMIUM]
            //            + data[index - 1][AALA_TP_ACCUMULATED_TOTAL_PREMIUM] :
            //            data[index][AALA_TP_TOTAL_PREMIUM]
            //            : 0;
            
            data[index][AALA_TP_AMOUNT_INVESTMENT_PORTION] =
            data[index][AALA_TP_TOTAL_PREMIUM]
            * data[index][AALA_TP_PCTG_INVESTMENT_PORTION];
            
            data[index][AALA_TP_AMOUNT_ACQUISITION_COST] =
            data[index][AALA_TP_TOTAL_PREMIUM]
            * data[index][AALA_TP_PCTG_ACQUISITION_COST];
            
            //withdrawal updated
            totalRegularPremiumWdwl += index > 0 ? data[index - 1][AALA_TOTAL_RP_WDWL] : 0;
            
            
            /**
             updated by Bill Fu
             TODO: if Basic premium <IDR 300mio/USD 300,000 then 50%
             else if Basic Premium >= IDR 300mio/USD 300,000 then 100%
             */
            //loyalty bonus
            //data[index][AALA_PCTG_LOYALTY_BONUS] = year <= maxLoyaltyBonusYear && (totalRegularPremiumWdwl <= (policy.annualPremium * 2)) ? pctgLoyaltyBonus[year - 1] : 0;
            
            double tempRate =strcmp(policy.currency, CURRENCY_IDR)==0 ? 200000000:20000;
            
            if (year ==maxLoyaltyBonusYear && totalRegularPremiumWdwl <= (policy.annualPremium * 2))
            {
                
                if (strcmp(policy.currency, CURRENCY_IDR)==0) {
                    if (year == 10)
                    {
                    }
                    data[index][AALA_PCTG_LOYALTY_BONUS] =policy.premium * policy.freq >=tempRate ? 0.5: 0.25;

                }else
                {
                    data[index][AALA_PCTG_LOYALTY_BONUS] =policy.premium * policy.freq >=tempRate ? 0.25: 0.125;

                }
            }else
                data[index][AALA_PCTG_LOYALTY_BONUS] = 0;
            
            /*
            if (year <= maxLoyaltyBonusYear) {
                data[index][AALA_PCTG_POTENSI_LOYALTY_BONUS] = (year > 1 ? pctgPotensiLoyaltyBonus[year - 1] : maxLoyaltyBonus);
            }else
                data[index][AALA_PCTG_POTENSI_LOYALTY_BONUS] = 0;
             */
            //data[index][AALA_PCTG_POTENSI_LOYALTY_BONUS] =  ? (year > 1 ? pctgPotensiLoyaltyBonus[year - 1] : maxLoyaltyBonus) : 0;
            
            
            data[index][AALA_LOYALTY_BONUS] = data[index][AALA_PCTG_LOYALTY_BONUS] * data[index][AALA_RP];
            
            
            //add by Knight  for 2016_08_10 AALA  HC,HD
            // modify by Marvin for 2016_09_16 spreadSheet
            
            if (year > 5) {
                if ((policy.aphStart == 0 || year < policy.aphStart) && totalRegularPremiumWdwl <= 0) {
                    if (strcmp(policy.currency,"USD")==0) {
                        
                        data[index][AALA_EXTRA_ALLOCATION] = 0.015 * data[index][AALA_RP];
                    }else{
                        data[index][AALA_EXTRA_ALLOCATION] = 0.03 * data[index][AALA_RP];
                    }
                } else {
                    data[index][AALA_EXTRA_ALLOCATION] = 0;
                }
                
            }else{
                
                data[index][AALA_EXTRA_ALLOCATION] = 0;
            }
            
            
            
            
            //       printf("-----%f----",data[index][AALA_EXTRA_ALLOCATION]);
            
            //            printf("%f\n",extraAllocation[year -1]);
            
            //     loyaltyBonusSum = loyaltyBonusSum + data[index][AALA_LOYALTY_BONUS];
            
            
            //          extraAllocationSum = extraAllocationSum + data[index][AALA_EXTRA_ALLOCATION];
            
            //           data[index][AALA_ANNUAL_LOYALTY_BOUNS] = loyaltyBonusSum + extraAllocationSum;
            
            
            //printf("%f",data[index][AALA_RP]);
           
            //marvin ma
            if (index == 119) {
            }
            data[index][AALA_RP_AMOUNT_INVESTMENT_PORTION] = data[index][AALA_RP]
            * data[index][AALA_RP_PCTG_INVESTMENT_PORTION] +
            data[index][AALA_EXTRA_ALLOCATION];
            
            
//            data[index][AALA_RP_AMOUNT_INVESTMENT_PORTION] = (data[index][AALA_RP] + data[index][AALA_EXTRA_ALLOCATION]) * data[index][AALA_RP_PCTG_INVESTMENT_PORTION]+ (index > 0 ? data[index -1][AALA_RP_AMOUNT_INVESTMENT_PORTION]:0);

            
            
            //end
            
            data[index][AALA_FOUNDATION_CHARGE_TINGKAT_INVESTASI] = (data[index][AALA_RP_AMOUNT_INVESTMENT_PORTION] + (index > 0 ? fmax(0, data[index - 1][AALA_RP_END_TOTAL]) : 0)) * data[index][AALA_PCTG_FOUNDATION_CHARGE];
            
            
            
            
            
            
            //double cost_sum = data[index][AALA_COST_OF_INSURANCE] + totalCor[year - 1] + data[index][AALA_BIAYA_POLIS_BULANAN]
            //+ data[index][AALA_FOUNDATION_CHARGE_TINGKAT_INVESTASI];
            
            //data[index][AALA_TOTAL_CHARGES] =
            //((index > 0 ? data[index - 1][AALA_TOTAL_MANFAAT_RISIKO_MENINGGAL] : 0) > 0 || durMonth <= noLapseGuaranteeMonth) ? cost_sum : 0;
            
            //Accumulated total charges
            //By Bill Fu
            //            APLAccTotalCharges = APLAccTotalCharges
            //            + data[index][AALA_TOTAL_CHARGES];
            //
            //            data[index][AALA_ACCUMULATED_TOTAL_CHARGES] =
            //            data[index][AALA_TOTAL_CHARGES] <= 0 ?
            //            0 : APLAccTotalCharges;
            
            // charges deduction
            //update By Bill Fu
            /*data[index][AALA_CHARGES_DEDUCT_FROM_TOPUP] = fmin(data[index][AALA_TOTAL_CHARGES] + data[index][AALA_APH_CHARGES]
             - fmin(0,
             (index > 0 ? data[index - 1][AALA_RP_ADI_TOTAL] : 0)), data[index][AALA_TP_AMOUNT_INVESTMENT_PORTION] + (durMonth > 1 ? data[index - 1][AALA_TP_END_TOTAL] : 0));*/
            
            
            
            /*data[index][AALA_CHARGES_DEDUCT_FROM_BASIC] =
             data[index][AALA_TOTAL_CHARGES]
             + data[index][AALA_APH_CHARGES]
             - data[index][AALA_CHARGES_DEDUCT_FROM_TOPUP];*/
            
            //end
            
            /*
             *During fund calculation
             */
            int k_t;
            /*
             * TP_ADI
             */
            double tp_adi_before_charge = 0;
            double ***funds_temp = funds;
            for (k_t = 0; k_t < policy.fundSize; k_t++)
            {
                int isFundIDR = !strcmp(policy.fundAllocations[k_t].currency, CURRENCY_IDR);
                double topupAllocation = policy.fundAllocations[k_t].topupAllocation;
                double investReturn = getFundMonthlyInvestmentReturn(policy.fundAllocations[k_t].groupId, scenario);
                double fundExRate = isSharedBasicIDR() ? (isFundIDR ? 1 : aala_exchangeRate) : isFundIDR ? 1 / aala_exchangeRate : 1;
                
                // XXX TP_ADI
                double adiValue = 0;
                adiValue = ((data[index][AALA_TP_RP] * topupAllocation) + (data[index][AALA_TP_AD_HOC_PREMIUM] * (data[index][AALA_TP_AD_HOC_PREMIUM] <= 0 ? 0 : topupAllocation))) * data[index][AALA_TP_PCTG_INVESTMENT_PORTION];
                
                adiValue /= fundExRate;
                
                if (index > 0)
                {
                    adiValue += funds[index - 1][k_t][AALA_TP_END];
                    funds_temp[index][k_t][AALA_TP_ADI] = (adiValue > 0 ? adiValue : 0);
                }
                else
                {
                    funds_temp[index][k_t][AALA_TP_ADI] = data[index][AALA_TP_AMOUNT_INVESTMENT_PORTION] <= 0 ? 0 : adiValue;
                }
                
                if (roundDoubleDown(funds[index][k_t][AALA_TP_ADI], -6) == 0)
                {
                    funds_temp[index][k_t][AALA_TP_ADI] = 0;
                }
                
                tp_adi_before_charge = tp_adi_before_charge + funds_temp[index][k_t][AALA_TP_ADI] * (1 + investReturn) * fundExRate;
                
            }
            
            
            //double pa_value_check = 0;
            
            /*
             * XXX RP_PCTG, TP_PCTG, CHARGES, NP, RP_ADI, RP_LOYALTY_BONUS
             */
            double rp_adi_before_charge =0;
            for (int k = 0; k < policy.fundSize; k++)
            {
                int isFundIDR = !strcmp(policy.fundAllocations[k].currency,
                                        CURRENCY_IDR);
                double regularAllocation =
                policy.fundAllocations[k].regularAllocation;
                double fundExRate = isSharedBasicIDR() ? (isFundIDR ? 1 : aala_exchangeRate) : isFundIDR ? 1 / aala_exchangeRate : 1;
                
                
                // XXX RP_ADI
                int validasi = 0;
                
                double adiValueRP =
                data[index][AALA_RP_AMOUNT_INVESTMENT_PORTION] * regularAllocation ;
                //                if (data[index][AALA_RP_AMOUNT_INVESTMENT_PORTION]>0) {
                //                    printf("%f\t",data[index][AALA_RP_AMOUNT_INVESTMENT_PORTION]);
                //                }
                
                if (isFundIDR)
                {
                    adiValueRP = adiValueRP * (isSharedBasicIDR()?1:aala_exchangeRate);
                    validasi = data[index][AALA_NLGI_VALIDASI] == 1;
                }
                else
                {
                    adiValueRP = adiValueRP /  (isSharedBasicIDR()?aala_exchangeRate:1);
                    validasi = data[index][AALA_NLGI_VALIDASI] == policy.noLapseGuaranteeYear;
                }
                
                adiValueRP = adiValueRP + (index > 0 ? funds[index - 1][k][AALA_RP_END] : 0);
                
                double tempAdi = index > 0 ? aala_getRP_ADI(year, adiValueRP, validasi, data[index - 1][AALA_RP_END_TOTAL],
                                                            data[index - 1][AALA_TP_END_TOTAL]) : (durMonth > noLapseGuaranteeMonth && adiValueRP <= 0) ? 0 : adiValueRP;
                //                if (adiValueRP>0) {
                //                    printf("%f\t",adiValueRP);
                //                    printf("%f\t",tempAdi);
                //                }
                //total calculation
                rp_adi_before_charge = rp_adi_before_charge + (index < 1 ? 0 : tempAdi * fundExRate);
                
                
            }
            // add by marvin ma
            

            
            //added by Bill Fu for V3.5 AALA
            //Withdrawal Updated
            int isWithdrawalYear_BA = fmod(durMonth, 12) == 0;
            double withdrawal_BA = 0;
            if (isWithdrawalYear_BA)
            {
                double value = fmin(tp_adi_before_charge, policy.topupWithdrawal[year - 1]);
                withdrawal_BA = isnan(value) ? 0 : value;
            }
            double total_tp_wdwl_basic = withdrawal_BA;
            double total_wdwl_basic = isWithdrawalYear_BA ? policy.topupWithdrawal[year - 1] : 0;
            double total_rp_wdwl_basic = isWithdrawalYear_BA ? total_wdwl_basic - total_tp_wdwl_basic + policy.regularWithdrawal[year - 1] : 0;
            
            
            //add by Bill FU
            double temp1 =0;
            if (durMonth <=1) {
                //temp1 =data[index][AALA_SUM_INSURED] -(data[index][AALA_RP_AMOUNT_INVESTMENT_PORTION]+data[index][AALA_TP_ADI_TOTAL]);
                //data[index][AALA_AD_SUM_INSURED] =data[index][AALA_SUM_INSURED];
                //data[index][AALA_ADJUST_SUM_INSURED] =data[index][AALA_AD_SUM_INSURED] -total_rp_wdwl_basic;
                tempadjust =data[index][AALA_SUM_INSURED]-total_rp_wdwl_basic;
                temp1 =data[index][AALA_SUM_INSURED] -(data[index][AALA_RP_AMOUNT_INVESTMENT_PORTION] );
            }else{
                //update by Bill Fu for TT446 item HnS & USD
                //temp1 =(data[index][AALA_SUM_INSURED] -total_rp_wdwl_basic) -((data[index - 1][AALA_RP_END_TOTAL]+data[index][AALA_RP_AMOUNT_INVESTMENT_PORTION]));
                //data[index][AALA_AD_SUM_INSURED] =data[index-1][AALA_ADJUST_SUM_INSURED];
                //data[index][AALA_ADJUST_SUM_INSURED] =data[index][AALA_AD_SUM_INSURED] -total_rp_wdwl_basic;
                //adjust[index][AALA_ADJUST_SUM_INSURED] =adjust[index-1][AALA_ADJUST_SUM_INSURED]-total_rp_wdwl_basic;
                //temp1 =data[index][AALA_ADJUST_SUM_INSURED] -((data[index - 1][AALA_RP_END_TOTAL]+data[index][AALA_RP_AMOUNT_INVESTMENT_PORTION]));
                //temp1 =data[index][AALA_ADJUST_SUM_INSURED] -rp_adi_before_charge;
                temp1 =tempadjust -rp_adi_before_charge;
                tempadjust =tempadjust-total_rp_wdwl_basic;
                //temp1 =data[index][AALA_SUM_INSURED] -((data[index - 1][AALA_RP_END_TOTAL_BEFORE]+data[index][AALA_RP_AMOUNT_INVESTMENT_PORTION])+tp_adi_before_charge);
            }
            if (index == 0)
            {
                
            }
            double d1 = fmax(temp1,(5*policy.premium *policy.freq));
            double d2 = data[index][AALA_RATE_COST_OF_INSURANCE] * d1;
            double d3 = d2/1000;
            
            // modify by charles 20160509
            //            double d4 = roundDoubleforAila(roundDoubleDown(d3,-6),-2);
            //            double d4 = roundDouble(d3,-2);
            // add by knight for TT1509
            double d4;
            
            double tempd4 = d3 * 100;
            double precious=0.0,inter=0;
            precious = modf(tempd4, &inter);
            if ((0.5 - precious) < 0.0001 && (0.5 - precious) > 0.0  && (0.5 - precious) > 0.000000001) {
                d4 = roundDoubleDown(d3,-2);
            }else{
                d4 = roundDouble(d3,-2);
            }
            
            //end
            
            
            //double d5 = roundDouble((roundDoubleDown(d3,-12)+roundDoubleUp(d3,-12))/2,-2);
            data[index][AALA_COST_OF_INSURANCE] = d4;
            
            //            printf("%f",data[index][AALA_COST_OF_INSURANCE]);
            
            //}
            //double cost_insured =roundDoubleUp(((data[index][AALA_RATE_COST_OF_INSURANCE] *
            //                                  (fmax(temp1,(5*policy.premium *policy.freq)))) / 1000), -2);
            
            double cost_sum = data[index][AALA_COST_OF_INSURANCE] + data[index][AALA_BIAYA_POLIS_BULANAN]
            + data[index][AALA_FOUNDATION_CHARGE_TINGKAT_INVESTASI];
            if (index == 0) {
                
            }
            /*
             *N = H + I + K + M
             *
             */
            data[index][AALA_TOTAL_CHARGES] = cost_sum;
            
            APLAccTotalCharges = APLAccTotalCharges
            + data[index][AALA_TOTAL_CHARGES];
            
            data[index][AALA_ACCUMULATED_TOTAL_CHARGES] =
            data[index][AALA_TOTAL_CHARGES] <= 0 ?
            0 : APLAccTotalCharges;
            
//            data[index][AALA_CHARGES_DEDUCT_FROM_BASIC] =(durMonth<(policy.noLapseGuaranteeYear*12))?(data[index][AALA_TOTAL_CHARGES]+data[index][AALA_APH_CHARGES]):(fmin(data[index][AALA_TOTAL_CHARGES]+data[index][AALA_APH_CHARGES],data[index][AALA_RP_AMOUNT_INVESTMENT_PORTION]+(durMonth > 1 ?data[index - 1][AALA_RP_END_TOTAL_BEFORE]:0)));
            
//            data[index][AALA_CHARGES_DEDUCT_FROM_BASIC] = minDouble(data[index][AALA_TOTAL_CHARGES], data[index][AALA_RP_END_TOTAL_BEFORE]);
            
            //            data[index][AALA_CHARGES_DEDUCT_FROM_TOPUP] =(policy.regularTopUp <=0 || year <=10) ? 0:(data[index][AALA_TOTAL_CHARGES] + data[index][AALA_APH_CHARGES]-data[index][AALA_CHARGES_DEDUCT_FROM_BASIC]);
            if (index == 482 && k_t == 0) {
                
            }
//            if (index == 0) {
//                data[index][AALA_CHARGES_DEDUCT_FROM_TOPUP] = 0;
//            }else{
//                data[index][AALA_CHARGES_DEDUCT_FROM_TOPUP] =(data[index - 1][AALA_TP_END_BEFORE] == 0) ? 0:(data[index][AALA_TOTAL_CHARGES] + data[index][AALA_APH_CHARGES]-data[index][AALA_CHARGES_DEDUCT_FROM_BASIC]);
//            }
//            data[index][AALA_CHARGES_DEDUCT_FROM_TOPUP] = minDouble(<#double x#>, <#double y#>)
            
            
            /*
             *During fund calculation
             */
            int k;
            double pa_value_check = 0;
            
            /*
             * XXX RP_PCTG, TP_PCTG, CHARGES, NP, RP_ADI, RP_LOYALTY_BONUS
             */
            //add by marvin ma
            
            
            /*
             *TP_END_BEFORE, RP_END_BEFORE
             */
            for (k = 0; k < policy.fundSize; k++)
            {
                int isFundIDR = !strcmp(policy.fundAllocations[k].currency,
                                        CURRENCY_IDR);
                double investReturn = getFundMonthlyInvestmentReturn(
                                                                     policy.fundAllocations[k].groupId, scenario);
                double fundExRate = isSharedBasicIDR() ? (isFundIDR ? 1 : aala_exchangeRate) : isFundIDR ? 1 / aala_exchangeRate : 1;
                
                // XXX TP_END_BEFORE
                //                funds[index][k][AALA_TP_END_BEFORE] = funds[index][k][AALA_TP_ADI]
                //                * (1 + investReturn);
                
                funds[index][k][AALA_TP_END_BEFORE] = data[index][AALA_TP_AMOUNT_INVESTMENT_PORTION] * policy.fundAllocations[k].topupAllocation / fundExRate + (index == 0 ? 0: funds[index - 1][k][AALA_TP_END]);
                
                // XXX RP_END_BEFORE
//                funds[index][k][AALA_RP_END_BEFORE] =
//                data[index][AALA_RP_ADI_TOTAL] < 0 ?
//                funds[index][k][AALA_RP_ADI] :
//                funds[index][k][AALA_RP_ADI]
//                * (1 + investReturn);
                //DH
                funds[index][k][AALA_RP_END_BEFORE] = data[index][AALA_RP_AMOUNT_INVESTMENT_PORTION] * policy.fundAllocations[k].regularAllocation / fundExRate + (index == 0? 0 : funds[index - 1][k][AALA_RP_END]);
                
                //total calculation
                data[index][AALA_TP_END_TOTAL_BEFORE] += (funds[index][k][AALA_TP_END_BEFORE] * fundExRate);
                if (index == 0 && k == 0)
                {
                    
                }
                data[index][AALA_RP_END_TOTAL_BEFORE] += (funds[index][k][AALA_RP_END_BEFORE] * fundExRate);
                
                
            }
            
            data[index][AALA_CHARGES_DEDUCT_FROM_BASIC] = minDouble(data[index][AALA_TOTAL_CHARGES], data[index][AALA_RP_END_TOTAL_BEFORE]);
            
            data[index][AALA_CHARGES_DEDUCT_FROM_TOPUP] = minDouble(data[index][AALA_TP_END_TOTAL_BEFORE], data[index][AALA_TOTAL_CHARGES] - data[index][AALA_CHARGES_DEDUCT_FROM_BASIC]);
            
            data[index][AALA_CHARGES_ON_BASIC] = minDouble(data[index][AALA_RP_END_TOTAL_BEFORE], data[index][AALA_CHARGES_TOTAL]);
            
            data[index][AALA_CHARGES_ON_TOPUP] = minDouble(data[index][AALA_TP_END_TOTAL_BEFORE], data[index][AALA_CHARGES_TOTAL] - data[index][AALA_CHARGES_ON_BASIC]);
            
            
            for (k = 0; k < policy.fundSize; k++)
            {
                int isFundIDR = !strcmp(policy.fundAllocations[k].currency,
                                        CURRENCY_IDR);
                double regularAllocation =
                policy.fundAllocations[k].regularAllocation;
                double topupAllocation =
                policy.fundAllocations[k].topupAllocation;
                double fundExRate = isSharedBasicIDR() ? (isFundIDR ? 1 : aala_exchangeRate) : isFundIDR ? 1 / aala_exchangeRate : 1;
                
                double investReturn = getFundMonthlyInvestmentReturn(
                                                                     policy.fundAllocations[k].groupId, scenario);
                
                // XXX RP_PCTG
                if (index == 0 && k == 2) {
                    
                }
                if (index == 1 && k == 2) {
                    
                }
                //add by marvin
                /*BX = o196 * AA 196 *fundExRate
                 *o196 = MIN(BV196,N196)
                 *N196 = total_charged
                 *AA196 = ?
                 */
                funds[index][k][AALA_RP_BEFORE_CHAGRGE] = (index <= 0 ? funds[index][k][AALA_RP_END] :
                                                           funds[index - 1][k][AALA_RP_END])
                * fundExRate + data[index][AALA_RP_AMOUNT_INVESTMENT_PORTION] * regularAllocation;
                
                
                funds[index][k][AALA_RP_PCTG_CHARGES] = (funds[index][k][AALA_RP_BEFORE_CHAGRGE] <= 0 || index <= 0 )?
                (index <= 0 ? regularAllocation :
                 funds[index-1][k][AALA_RP_BEFORE_CHAGRGE]) :
                funds[index][k][AALA_RP_BEFORE_CHAGRGE] /*/fundExRate*//(data[index - 1][AALA_RP_END_TOTAL] + data[index][AALA_RP_AMOUNT_INVESTMENT_PORTION]);
                if (index == 0 && k == 0) {
                    
                }
                
                
                
                if (isFundIDR)
                {
                    
                    funds[index][k][AALA_CHARGES] = minDouble(data[index][AALA_RP_END_TOTAL_BEFORE], data[index][AALA_TOTAL_CHARGES]) / fundExRate * funds[index][k][AALA_RP_PCTG_CHARGES];
                    
                }else
                {
                    funds[index][k][AALA_CHARGES] = minDouble(data[index][AALA_RP_END_TOTAL_BEFORE], data[index][AALA_TOTAL_CHARGES]) * funds[index][k][AALA_RP_PCTG_CHARGES] /fundExRate;
                    
                    
                    //                    if (index == 0) {
                    //
                    //                    }else
                    //                    {
                    //                        funds[index][k][AALA_CHARGES] = minDouble(totalRPBeforeCharge, data[index][AALA_TOTAL_CHARGES]) * funds[index][k][AALA_RP_PCTG_CHARGES];
                    //
                    //                    }
                    
                }
                
                //                funds[index][k][AALA_RP_PCTG_CHARGES] =
                //                durMonth == 1 || data[index - 1][AALA_RP_END_TOTAL] <= 0 ? regularAllocation :
                //                (funds[index - 1][k][AALA_RP_END] * fundExRate + data[index][AALA_RP_AMOUNT_INVESTMENT_PORTION] * regularAllocation)
                //                / (data[index - 1][AALA_RP_END_TOTAL] + data[index][AALA_RP_AMOUNT_INVESTMENT_PORTION]);
                
                // XXX CHARGES
                
                //                double chargesValue = data[index][AALA_CHARGES_DEDUCT_FROM_BASIC]
                //                * funds[index][k][AALA_RP_PCTG_CHARGES];
                
                //funds[index][k][AALA_CHARGES] = chargesValue / fundExRate;
                
                // XXX NP
                double npaValue = data[index][AALA_RP_AMOUNT_INVESTMENT_PORTION]
                * regularAllocation;
                
                if (!isFundIDR)
                {
                    npaValue = npaValue / fundExRate;
                }
                
                npaValue = npaValue
                + (index > 0 ? funds[index - 1][k][AALA_RP_END] : 0);
                
                npaValue = (
                            npaValue > funds[index][k][AALA_CHARGES] ?
                            0 : funds[index][k][AALA_CHARGES] - npaValue);
                
                funds[index][k][AALA_NP_ACC] =
                (year <= policy.noLapseGuaranteeYear
                 && data[index][AALA_NLGI_VALIDASI] == 0) ?
                0 : (npaValue <= 0 ? 0 : npaValue);
                
                // XXX TP_PCTG
                funds[index][k][AALA_TP_PCTG_CHARGES] =
                durMonth == 1
                || data[index - 1][AALA_TP_END_TOTAL] <= 0 ?
                topupAllocation :
                (funds[index - 1][k][AALA_TP_END] * fundExRate
                 + (data[index][AALA_TP_RP]
                    * topupAllocation
                    * data[index][AALA_TP_PCTG_INVESTMENT_PORTION]))
                / (data[index - 1][AALA_TP_END_TOTAL]
                   + data[index][AALA_TP_RP]
                   * data[index][AALA_TP_PCTG_INVESTMENT_PORTION]);
                
                // XXX RP_ADI
                int validasi = 0;
                
                if (index == 0 && k == 0) {
                    
                }
                double adiValueRP =
                data[index][AALA_RP_AMOUNT_INVESTMENT_PORTION]
                * regularAllocation;
                
                if (isFundIDR)
                {
                    adiValueRP = adiValueRP * (isSharedBasicIDR()?1:aala_exchangeRate) - funds[index][k][AALA_CHARGES];
                    validasi = data[index][AALA_NLGI_VALIDASI] == 1;
                }
                else
                {
                    adiValueRP = adiValueRP /  (isSharedBasicIDR()?aala_exchangeRate:1)- funds[index][k][AALA_CHARGES];
                    validasi = data[index][AALA_NLGI_VALIDASI]
                    == policy.noLapseGuaranteeYear;
                }
                
                adiValueRP = adiValueRP + (index > 0 ? funds[index - 1][k][AALA_RP_END] : 0);
                
                //marvin ma change
                if (index == 83 && k == 2) {
                    
                }
                funds[index][k][AALA_RP_ADI] = adiValueRP;
               
                funds[index][k][AALA_BASIC_AV_AFTER_INVEST] = funds[index][k][AALA_RP_ADI] * (1 + investReturn);
                if (isSharedBasicIDR() && !isFundIDR) {
                    data[index][AALA_BASIC_AV_AFTER_INVEST_TOTAL] += funds[index][k][AALA_BASIC_AV_AFTER_INVEST]*10000;

                }else if (!isSharedBasicIDR() && isFundIDR)
                {
                    data[index][AALA_BASIC_AV_AFTER_INVEST_TOTAL] += funds[index][k][AALA_BASIC_AV_AFTER_INVEST]/10000;

                }else
                {
                    data[index][AALA_BASIC_AV_AFTER_INVEST_TOTAL] += funds[index][k][AALA_BASIC_AV_AFTER_INVEST];

                }
                
                //                funds[index][k][AALA_RP_ADI] = index > 0 ?
                //                aala_getRP_ADI(year, adiValueRP, validasi, data[index - 1][AALA_RP_END_TOTAL],
                //                               data[index - 1][AALA_TP_END_TOTAL]) : (durMonth > noLapseGuaranteeMonth && adiValueRP <= 0) ? 0 : adiValueRP;
                
                //total calculation
                data[index][AALA_CHARGES_TOTAL] = data[index][AALA_CHARGES_TOTAL] + (funds[index][k][AALA_CHARGES] * fundExRate);
                data[index][AALA_RP_ADI_TOTAL] = data[index][AALA_RP_ADI_TOTAL] + (funds[index][k][AALA_RP_ADI] * fundExRate);
                data[index][AALA_NP_ACC_TOTAL] = data[index][AALA_NP_ACC_TOTAL]
                + (index > 0 ? (data[index - 1][AALA_TP_END_TOTAL]
                                + data[index - 1][AALA_RP_END_TOTAL] > 0 ?
                                funds[index][k][AALA_NP_ACC] * fundExRate :0) :0);
                
                pa_value_check = pa_value_check
                + funds[index][k][AALA_RP_PCTG_CHARGES];
                pa_value_check = pa_value_check
                + funds[index][k][AALA_TP_PCTG_CHARGES];
                
            }
            
            
            
            
            // funds charge on basic av and on TOP AV
//            for (k = 0; k < policy.fundSize; k++)
//            {
//                int isFundIDR = !strcmp(policy.fundAllocations[k].currency, CURRENCY_IDR);
//                double investReturn = getFundMonthlyInvestmentReturn(policy.fundAllocations[k].groupId, scenario);
//                double BasicAllocation = policy.fundAllocations[k].regularAllocation;
//                double topupAllocation = policy.fundAllocations[k].topupAllocation;
//                double fundExRate = isSharedBasicIDR() ? (isFundIDR ? 1 : aala_exchangeRate) : isFundIDR ? 1 / aala_exchangeRate : 1;
//                
//                funds[index][k][AALA_BASIC_CHARGES] = data[index][AALA_CHARGES_ON_BASIC] * BasicAllocation*fundExRate;
//                data[index][AALA_CHARGES_ON_BASIC_TOTAL] += funds[index][k][AALA_BASIC_CHARGES];
//                
//                funds[index][k][AALA_TOPUP_CHARGES] = data[index][AALA_CHARGES_ON_TOPUP] * topupAllocation*fundExRate;
//                data[index][AALA_CHARGES_ON_TOPUP_TOTAL] += funds[index][k][AALA_TOPUP_CHARGES];
//            }
            
            
            /*
             * TP_ADI
             */
            double tp_adi = 0;
            for (k = 0; k < policy.fundSize; k++)
            {
                int isFundIDR = !strcmp(policy.fundAllocations[k].currency, CURRENCY_IDR);
                double investReturn = getFundMonthlyInvestmentReturn(policy.fundAllocations[k].groupId, scenario);
                double topupAllocation = policy.fundAllocations[k].topupAllocation;
                double fundExRate = isSharedBasicIDR() ? (isFundIDR ? 1 : aala_exchangeRate) : isFundIDR ? 1 / aala_exchangeRate : 1;
                
                // XXX TP_ADI
                double adiValue = 0;
                adiValue = ((data[index][AALA_TP_RP] * topupAllocation) + (data[index][AALA_TP_AD_HOC_PREMIUM] * (data[index][AALA_TP_AD_HOC_PREMIUM] <= 0 ? 0 : topupAllocation))) * data[index][AALA_TP_PCTG_INVESTMENT_PORTION];
                
                adiValue /= fundExRate;
                //changed by marvin ma
                /*
                 * GL498 = FM498 - FZ498
                 * FM498 = TP Acc Value before charge
                 * FZ498 = P498 * EC498 *fundExRate
                 * P498  = MIN (FY498,N498-O498)
                 * FY = TP ACC value before charge
                 */
                if (index == 482 && k == 0) {
                    //minDouble(FY498, data[index][AALA_TOTAL_CHARGES] - )
                }
                if (index > 0)
                {
                    adiValue += funds[index - 1][k][AALA_TP_END];
                    double deductValue = data[index][AALA_CHARGES_DEDUCT_FROM_TOPUP] * funds[index][k][AALA_TP_PCTG_CHARGES] / fundExRate;
                    funds[index][k][AALA_TP_ADI] = (adiValue > 0 ? adiValue : 0) - deductValue;
                }
                else
                {
                    funds[index][k][AALA_TP_ADI] = data[index][AALA_TP_AMOUNT_INVESTMENT_PORTION] <= 0 ? 0 : (adiValue - (data[index][AALA_CHARGES_DEDUCT_FROM_TOPUP] * funds[index][k][AALA_TP_PCTG_CHARGES] / fundExRate));
                }
                
                if (roundDoubleDown(funds[index][k][AALA_TP_ADI], -6) == 0)
                {
                    funds[index][k][AALA_TP_ADI] = 0;
                }
                //HK
                if (index == 71 && k == 0) {
                    
                }
                if (index == 71 && k == 2) {
                    
                }
                funds[index][k][AALA_TOPUP_AV_AFTER_INVEST] = funds[index][k][AALA_TP_ADI] * (1 + investReturn);
                //marvin ma test
                if (!isSharedBasicIDR() && isFundIDR)
                {
                    data[index][AALA_TOPUP_AV_AFTER_INVEST_TOTAL] += funds[index][k][AALA_TOPUP_AV_AFTER_INVEST]/10000;

                } else if(isSharedBasicIDR() && !isFundIDR)
                {
                    data[index][AALA_TOPUP_AV_AFTER_INVEST_TOTAL] += funds[index][k][AALA_TOPUP_AV_AFTER_INVEST] * 10000;

                }
                else
                {
                    data[index][AALA_TOPUP_AV_AFTER_INVEST_TOTAL] += funds[index][k][AALA_TOPUP_AV_AFTER_INVEST];

                }
                
                //total calculation
                data[index][AALA_TP_ADI_TOTAL] = data[index][AALA_TP_ADI_TOTAL] + funds[index][k][AALA_TP_ADI] * fundExRate;
                
                //Wtihdrawal Updated
                tp_adi += funds[index][k][AALA_TP_ADI] * (1 + investReturn) * fundExRate;
                
            }
            
            
            //Withdrawal Updated
            int isWithdrawalYear = fmod(durMonth, 12) == 0;
            double withdrawal = 0;
            if (isWithdrawalYear)
            {
                double value = fmin(tp_adi, policy.topupWithdrawal[year - 1]);
                withdrawal = isnan(value) ? 0 : value;
            }
            data[index][AALA_TOTAL_TP_WDWL] = withdrawal;
            data[index][AALA_TOTAL_WDWL] = isWithdrawalYear ? policy.topupWithdrawal[year - 1] : 0;
            if (index == 59) {
                
            }
            data[index][AALA_TOTAL_RP_WDWL] = isWithdrawalYear ? data[index][AALA_TOTAL_WDWL] - data[index][AALA_TOTAL_TP_WDWL] + policy.regularWithdrawal[year - 1] : 0;
            sumTotalRPWdrl += data[index][AALA_TOTAL_RP_WDWL];
            
            //            change 0 to 0.0000001
            //            fix [TT#240][Mork Mo][Agency]
            // add by marvin ma
//            for (int i = 0; i < sizeof(policy.adhocTopUp)/sizeof(policy.adhocTopUp[0]); i++) {
//                
//            }
            if (year < policy.regularWithdrawStartYear && sumTotalRPWdrl > 0.0000001)
            {
                aala_allowedWdrlPremium = FALSE;
            }
            
//            /*
//             *TP_END_BEFORE, RP_END_BEFORE
//             */
//            for (k = 0; k < policy.fundSize; k++)
//            {
//                int isFundIDR = !strcmp(policy.fundAllocations[k].currency,
//                                        CURRENCY_IDR);
//                double investReturn = getFundMonthlyInvestmentReturn(
//                                                                     policy.fundAllocations[k].groupId, scenario);
//                double fundExRate = isSharedBasicIDR() ? (isFundIDR ? 1 : aala_exchangeRate) : isFundIDR ? 1 / aala_exchangeRate : 1;
//                
//                // XXX TP_END_BEFORE
////                funds[index][k][AALA_TP_END_BEFORE] = funds[index][k][AALA_TP_ADI]
////                * (1 + investReturn);
//                
//                funds[index][k][AALA_TP_END_BEFORE] = data[index][AALA_TP_AMOUNT_INVESTMENT_PORTION] * policy.fundAllocations[k].topupAllocation * fundExRate + (index == 0 ? 0: funds[index][k][AALA_TP_END]);
//                
//                // XXX RP_END_BEFORE
//                funds[index][k][AALA_RP_END_BEFORE] =
//                data[index][AALA_RP_ADI_TOTAL] < 0 ?
//                funds[index][k][AALA_RP_ADI] :
//                funds[index][k][AALA_RP_ADI]
//                * (1 + investReturn);
//                
//                //total calculation
//                data[index][AALA_TP_END_TOTAL_BEFORE] += (funds[index][k][AALA_TP_END_BEFORE] / fundExRate);
//                
//                data[index][AALA_RP_END_TOTAL_BEFORE] =
//                data[index][AALA_RP_END_TOTAL_BEFORE]
//                + (funds[index][k][AALA_RP_END_BEFORE]
//                   * fundExRate);
//                
//                
//            }
            
           
            /*
             * RP_WITHDRAWAL, RP_END, TP_WITHDRAWAL, TP_END
             */
            for (k = 0; k < policy.fundSize; k++)
            {
                int isFundIDR = !strcmp(policy.fundAllocations[k].currency,
                                        CURRENCY_IDR);
                /*
                 * fund的rate不对
                 *
                 */
                if (index == 0 && k == 0) {
                    
                }
                double investReturn = getFundMonthlyInvestmentReturn(
                                                                     policy.fundAllocations[k].groupId, scenario);
                //                double investReturn = roundDoubleDown(investReturnValue, -13);
                double regularAllocation =
                policy.fundAllocations[k].regularAllocation;
                double topUpAllocation =
                policy.fundAllocations[k].topupAllocation;
                
                double fundExRate = isSharedBasicIDR() ? (isFundIDR ? 1 : aala_exchangeRate) : isFundIDR ? 1 / aala_exchangeRate : 1;
                if (index == 71 && k ==0) {
                    
                }
                if (index == 83) {
                    
                }
                
                /*
                 *AL195 = CV195/fundExRate/DH195
                 *CV195 = funds[index][k][AALA_RP_ADI] * (1 + investReturn)
                 *DH195 =
                 */
                // XXX PCTG WITHDRAWAL
                if (index == 0)
                {
                    funds[index][k][AALA_FEUL_PCTG_CHARGES] = regularAllocation;
                    funds[index][k][AALA_WITHDRAWAL_ALLOCATION_RP] = regularAllocation;
                    funds[index][k][AALA_WITHDRAWAL_ALLOCATION_TP] = topUpAllocation;
                }
                else
                {
                    //update by Bill Fu not sure if correct.TT446 USD pdf
                    

                    //changed by marvin ma
                    //                    paFeulpp =
                    //                    (data[index - 1][AALA_RP_END_TOTAL] <= 0) ?
                    //                    regularAllocation :
                    //                    ((funds[index - 1][k][AALA_RP_END]
                    //                      )
                    //                     * fundExRate)
                    //                    / (data[index - 1][AALA_RP_END_TOTAL]
                    //                       );
                    
                    // RP withdraw allocation
                    if (data[index][AALA_BASIC_AV_AFTER_INVEST_TOTAL] == 0) {
                        funds[index][k][AALA_WITHDRAWAL_ALLOCATION_RP] = funds[index - 1][k][AALA_WITHDRAWAL_ALLOCATION_RP];
                    }else{
                        if (isSharedBasicIDR() && !isFundIDR) {
                            funds[index][k][AALA_WITHDRAWAL_ALLOCATION_RP] = funds[index][k][AALA_RP_ADI] * (1 + investReturn)* 10000 /data[index][AALA_BASIC_AV_AFTER_INVEST_TOTAL];

                        }else if (!isSharedBasicIDR() && isFundIDR)
                        {
                            funds[index][k][AALA_WITHDRAWAL_ALLOCATION_RP] = funds[index][k][AALA_RP_ADI] * (1 + investReturn)/ 10000 /data[index][AALA_BASIC_AV_AFTER_INVEST_TOTAL];

                        }else
                        {
                            funds[index][k][AALA_WITHDRAWAL_ALLOCATION_RP] = funds[index][k][AALA_RP_ADI] * (1 + investReturn) / data[index][AALA_BASIC_AV_AFTER_INVEST_TOTAL];

                        }
                    }
                    
                    // TP withdraw allocation
                    if (data[index][AALA_TOPUP_AV_AFTER_INVEST_TOTAL] == 0) {
                        funds[index][k][AALA_WITHDRAWAL_ALLOCATION_TP] = funds[index - 1][k][AALA_WITHDRAWAL_ALLOCATION_TP];
                    }else{
                        if (!isSharedBasicIDR() && isFundIDR) {
                            funds[index][k][AALA_WITHDRAWAL_ALLOCATION_TP] = funds[index][k][AALA_TP_ADI] * (1 + investReturn)/ 10000 / data[index][AALA_TOPUP_AV_AFTER_INVEST_TOTAL];
                        } else if (isSharedBasicIDR() && !isFundIDR)
                        {
                            //need to do
                            funds[index][k][AALA_WITHDRAWAL_ALLOCATION_TP] = funds[index][k][AALA_TP_ADI] * (1 + investReturn) * 10000 / data[index][AALA_TOPUP_AV_AFTER_INVEST_TOTAL];

                        }
                        else
                        {
                            funds[index][k][AALA_WITHDRAWAL_ALLOCATION_TP] = funds[index][k][AALA_TP_ADI] * (1 + investReturn) / data[index][AALA_TOPUP_AV_AFTER_INVEST_TOTAL];

                        }
                    }
                    
                    funds[index][k][AALA_FEUL_PCTG_CHARGES] =
                    (data[index][AALA_TOTAL_TP_WDWL] == 0 || data[index - 1][AALA_RP_END_TOTAL] <= 0) ? regularAllocation :
                    funds[index][k][AALA_TP_END_BEFORE] * fundExRate / data[index][AALA_TP_END_TOTAL_BEFORE];
                    
                }
                if (index == 83 && k == 2) {
                    
                }
                /*
                 *AX75 = Q75 * AL75 *AX11
                 *
                 */
               
                funds[index][k][AALA_RP_WITHDRAWAL] = data[index][AALA_TOTAL_RP_WDWL]* funds[index][k][AALA_WITHDRAWAL_ALLOCATION_RP] / fundExRate;
                
                APLWDRLTotal = APLWDRLTotal
                + funds[index][k][AALA_RP_WITHDRAWAL];
                
                // XXX TP_WITHDRAWAL
                funds[index][k][AALA_TP_WITHDRAWAL] =
                data[index][AALA_TOTAL_TP_WDWL]
                * (funds[index][k][AALA_WITHDRAWAL_ALLOCATION_TP]
                   / fundExRate);
                
                
                APLWDRLTotal = APLWDRLTotal
                + funds[index][k][AALA_TP_WITHDRAWAL];
                
                // XXX RP_LOYALTY_BONUS
                //marvin ma changed
                if (index == 119 && k ==2)
                {
                }
                //marvin ma changed
                /*
                 1.valid APH
                 year < policy.aphStart ? 0 : 1
            
                 2.valid withDrwal
                 policy.regularWithdrawStartYear
                 */
                int validAPH = (index <= 0) ? 0:( data[index -1][AALA_APH] ==1 ? 1 :(data[index][AALA_APH] == 0 ? 0 : 1));
                //totalRegularPremiumWdwl += index > 0 ? data[index - 1][AALA_TOTAL_RP_WDWL] : 0;
                int validWithDrawl = (isWithdrawalYear && (totalRegularPremiumWdwl > policy.annualPremium * 2) ? 1 : 0);
                double pctg_loyalty_bonus = 0.0;
                if (fmod(durMonth, 12) == 0 && (validWithDrawl +validAPH) == 0)
                {
                    pctg_loyalty_bonus = data[index][AALA_PCTG_LOYALTY_BONUS];
                }else
                {
                    data[index][AALA_PCTG_LOYALTY_BONUS] = pctg_loyalty_bonus;
                }
                
               
                
                funds[index][k][AALA_RP_LOYALTY_BONUS] =pctg_loyalty_bonus * policy.premium * policy.freq;
                if (index ==119 && k ==2) {
                }
                
                // XXX RP_END
                //need to do: marvin ma
                if (index == 83 && k == 2)
                {
                    
                }
                funds[index][k][AALA_RP_END] = funds[index][k][AALA_RP_ADI] * (1 + investReturn)
                - funds[index][k][AALA_RP_WITHDRAWAL]
                + funds[index][k][AALA_RP_LOYALTY_BONUS] * regularAllocation * (1/fundExRate);//DK 124
                
                 
                
                
                // XXX TP_END
                if (index >= 71 && k == 0) {
                    
                }
                funds[index][k][AALA_TP_END] = funds[index][k][AALA_TP_ADI] * (1 + investReturn) - (index == 0 ? 0 : funds[index][k][AALA_TP_WITHDRAWAL]);
                
                
                
                //total calculation
                //                data[index][AALA_TP_END_TOTAL] = data[index][AALA_TP_END_TOTAL] + (funds[index][k][AALA_TP_END] * fundExRate);
                if (index >= 71  && k == 1) {
                    
                }
                data[index][AALA_TP_END_TOTAL] = data[index][AALA_TP_END_TOTAL] + roundDouble(funds[index][k][AALA_TP_END] * fundExRate , -5);
                //add by marvin ma
                if (funds[index][k][AALA_TP_END] < 0) {
                    data[index][AALA_TP_END_TOTAL] = 0.0;
                }
                if (index == 0) {
                    
                }
                data[index][AALA_RP_END_TOTAL] = data[index][AALA_RP_END_TOTAL] + funds[index][k][AALA_RP_END] * fundExRate;
                
                data[index][AALA_RP_LOYALTHY_BONUS_TOTAL] = data[index][AALA_RP_LOYALTHY_BONUS_TOTAL] + (funds[index][k][AALA_RP_LOYALTY_BONUS] * fundExRate);
                
                
            }
            
            
            
            
            data[index][AALA_RP_END_TOTAL] = data[index][AALA_RP_END_TOTAL];
            data[index][AALA_RP_LOYALTHY_BONUS_TOTAL] = /*(data[index][AALA_NLGI_VALIDASI] == 0 && data[index][AALA_RP_END_TOTAL] < 0) ? 0 :*/ data[index][AALA_RP_LOYALTHY_BONUS_TOTAL];
            checkWithdrawal = APLWDRLTotal > data[index][AALA_TP_END_TOTAL_BEFORE];
            
            APLWDRLTotal = 0;
            
            /*
             * After fund calculation
             */
            data[index][AALA_SC_RP] = data[index][AALA_RP_END_TOTAL]
            * data[index][AALA_PCTG_SC_RP];
            data[index][AALA_ESTIMASI_NILAI_TEBUS] =
            data[index][AALA_RP_END_TOTAL] - data[index][AALA_SC_RP]
            + data[index][AALA_TP_END_TOTAL];
            
            int isDeathBenefitValid = (data[index][AALA_ESTIMASI_NILAI_TEBUS]
                                       <= 0 && year > policy.noLapseGuaranteeYear);
            //update by Bill Fu
            //temp is Sum at Risk Death new requirment at 2015/04/30
            //double adjustSA =data[index][AALA_SUM_INSURED] -(durMonth == 1 ? 0 : data[index-1][AALA_TOTAL_WDWL])+data[index][AALA_TP_AD_HOC_PREMIUM];
            //double temp =data[index][AALA_SUM_INSURED] -(data[index][AALA_RP_END_TOTAL]+data[index][AALA_TP_END_TOTAL]);
            //update by Bill Fu for V3.5
            
            //double temp =data[index][AALA_SUM_INSURED] - total_rp_wdwl_basic - data[index][AALA_RP_END_TOTAL];
            double temp = tempadjust-data[index][AALA_RP_END_TOTAL];
            
            if (index == 419) {

            }
            //IH = DU + HX
            if (isnan(data[index][AALA_RP_END_TOTAL]) || isnan(data[index][AALA_TP_END_TOTAL]))
            {
                if (index == 0)
                {
                    data[index][AALA_TOTAL_MANFAAT_RISIKO_MENINGGAL] = policy.sumAssured * lien[insuredAge];
                    
                }else
                {
                    if (data[index -1][AALA_TOTAL_MANFAAT_RISIKO_MENINGGAL] < policy.sumAssured * lien[insuredAge])
                    {
                        data[index][AALA_TOTAL_MANFAAT_RISIKO_MENINGGAL] = 0;
                    }else
                    {
                        data[index][AALA_TOTAL_MANFAAT_RISIKO_MENINGGAL] = policy.sumAssured * lien[insuredAge];
                        
                    }
                    
                }
                
            }else
            {
                if (index > 0)
                {
                    if (data[index -1][AALA_RP_END_TOTAL] < 0 && roundDouble(data[index -1][AALA_TOTAL_MANFAAT_RISIKO_MENINGGAL], 0) < policy.sumAssured * lien[insuredAge])
                    {
                        data[index][AALA_TOTAL_MANFAAT_RISIKO_MENINGGAL] = 0;
                    }else
                    {
                        data[index][AALA_TOTAL_MANFAAT_RISIKO_MENINGGAL] = (policy.sumAssured * lien[insuredAge] +data[index][AALA_RP_END_TOTAL] + data[index][AALA_TP_END_TOTAL]);
                        
                    }
                }else
                {
                    data[index][AALA_TOTAL_MANFAAT_RISIKO_MENINGGAL] = (policy.sumAssured * lien[insuredAge] +data[index][AALA_RP_END_TOTAL] + data[index][AALA_TP_END_TOTAL]);
                    
                }
                
                
            }
        
        
            /*
            data[index][AALA_TOTAL_MANFAAT_RISIKO_MENINGGAL] =
            aala_getManfaatRisikoMeninggal(year,
                                           //data[index][AALA_SUM_INSURED],
                                           fmax(temp,(5*policy.premium *policy.freq)),
                                           //fmax(adjustSA,(5*policy.premium *policy.freq)),
                                           data[index][AALA_ESTIMASI_NILAI_TEBUS],
                                           data[index][AALA_RP_END_TOTAL],
                                           data[index][AALA_TP_END_TOTAL], lien[insuredAge]);
            */
            
            
            //data[index][AALA_COST_OF_INSURANCE] = roundDouble(((data[index][AALA_RATE_COST_OF_INSURANCE] *
            //                                                     (durMonth<=1?temp :max(temp,(5*policy.premium *policy.freq)))) / 1000), -2);
            
            // Death Benefit of ADB & Indemnity
            double aalaAdb =0;
            if (strcmp(policy.currency,"USD")==0) {
                aalaAdb =adb[insuredAge]/aala_exchangeRate;
            }else
                aalaAdb =adb[insuredAge];
            
            data[index][AALA_TOTAL_MANFAAT_RISIKO_MENINGGAL_ADB] =
            //            aala_getDeathBenefit(isDeathBenefitValid, adb[insuredAge],
            //                                sumAssured, data[index][AALA_RP_END_TOTAL],
            //                                data[index][AALA_TP_END_TOTAL], lien[insuredAge], 1);
            aala_getDeathBenefit(isDeathBenefitValid, aalaAdb,
                                 //sumAssured,
                                 fmax(temp,(5*policy.premium *policy.freq)),
                                 //fmax(adjustSA,(5*policy.premium *policy.freq)),
                                 data[index][AALA_RP_END_TOTAL],
                                 data[index][AALA_TP_END_TOTAL], lien[insuredAge], 1);
            //add by marvin ma
            //citibank need to + IF
            data[index][AALA_TOTAL_MANFAAT_RISIKO_MENINGGAL_ADB] = (insuredAge >= 70 ? 0 :
            minDouble(policy.sumAssured * lien[insuredAge], adb[insuredAge])) + policy.sumAssured *lien[insuredAge] + (data[index][AALA_RP_END_TOTAL] + data[index][AALA_TP_END_TOTAL]);
            
            //end
            
            double aalaIndemnity =0;
            if (strcmp(policy.currency,"USD")==0) {
                aalaIndemnity =indemnity[insuredAge]/aala_exchangeRate;
            }else
                aalaIndemnity =indemnity[insuredAge];
            //add by marvin ma
            data[index][AALA_TOTAL_MANFAAT_DOUBLE_INDEMNITY] =
            (insuredAge >= 70 ? 0 :
             minDouble(policy.sumAssured * lien[insuredAge], adb[insuredAge]))*2 + policy.sumAssured *lien[insuredAge] + (data[index][AALA_RP_END_TOTAL] + data[index][AALA_TP_END_TOTAL]);
            //end
            
            //DU, DV, DW, DX
            data[index][AALA_RP_ADI_TOTAL_CHECK] =
            year == 0 ? 100000 :
            data[index][AALA_RP_ADI_TOTAL] <= 0 ? durMonth : 99999;
            
            data[index][AALA_SUM_AT_RISK] =
            data[index][AALA_TOTAL_MANFAAT_RISIKO_MENINGGAL] == 0 ?
            0 :
            data[index][AALA_TOTAL_MANFAAT_RISIKO_MENINGGAL]
            - data[index][AALA_ESTIMASI_NILAI_TEBUS];
            
            data[index][AALA_RP_END_TOTAL_CHECK] =
            year <= 1 ? 0 : data[index][AALA_RP_END_TOTAL] <= 0 ? 1 : 0;
            
            data[index][AALA_PCTG_ALLOCATION_CHECK] = pa_value_check - 2;
            data[index][AALA_TOTAL_ACC_FUND] = data[index][AALA_RP_END_TOTAL]
            + data[index][AALA_TP_END_TOTAL];
            data[index][AALA_ROUNDED_TOTAL_ACC_FUND] = roundDoubleDown(
                                                                       year <= policy.noLapseGuaranteeYear ?
                                                                       data[index][AALA_TOTAL_ACC_FUND] :
                                                                       data[index][AALA_TOTAL_ACC_FUND] <= 0 ?
                                                                       0 : data[index][AALA_TOTAL_ACC_FUND], 0);
            data[index][AALA_MAXIMUM_WITHDRAWAL] =
            data[index][AALA_RP_END_TOTAL_BEFORE]
            - data[index][AALA_SC_RP]
            + data[index][AALA_TP_END_TOTAL_BEFORE];
            
            
        }
    }
    
//    free3DDoubleArray(funds, policy.coverageTerm * 12, policy.fundSize);
    free(lien);
    free(adb);
    free(indemnity);
    free(coiDiscount);
    free(coi);
    //free(totalCor);
    free(foundationCharge);
    free(pctgLoyaltyBonus);
    free(pctgPotensiLoyaltyBonus);
}

double **aala_calculateBasic(int scenario, int *row, int *col)
{
    double **data = make2DDoubleArray(policy.coverageTerm * 12, AALA_CAL_SIZE);
    
    double *aphCharge = aala_getAphCharge(policy.prefix, NULL);
    double *premiumAllocation = aala_getPremiumAllocation(policy.prefix, NULL);
    double *topupAllocation = aala_getTopupAllocation(policy.prefix, NULL);
    double *surrenderCharge = aala_getSurrenderCharge(policy.prefix, NULL);
    double *monthlyPolicyFee = aala_getMonthlyPolicyFee(policy.prefix, NULL);
    
    if (row != NULL)
    {
        *row = policy.coverageTerm * 12;
    }
    if (col != NULL)
    {
        *col = AALA_CAL_SIZE;
    }
    
    int i, j;
    
    for (i = 0; i < policy.coverageTerm; i++)
    {
        const int year = i + 1;
        const int insuredAge = insured.age + i;
        const int payorAge = payor.age + i;
        
        for (j = 0; j < 12; j++)
        {
            const int index = i * 12 + j;
            //			const int month = j + 1;
            const int durMonth = index + 1;
            
            data[index][AALA_BEGINNING_MONTH] = durMonth;
            data[index][AALA_POLICY_YEAR] = year;
            data[index][AALA_AGE_INSURED] = insuredAge;
            data[index][AALA_SUM_INSURED] = policy.sumAssured;
            
            data[index][AALA_APH] =
            (year < policy.aphStart || policy.aphStart == 0) ? 0 :
            year <= policy.aphEnd ? 1 : 0;
            
            if (index == 108)
            {

            }
            //need to change : marvin ma
            data[index][AALA_RP] = aala_getBasicPremium(durMonth,
                                                        (int) data[index][AALA_APH], policy.premium);
            data[index][AALA_RP_TOTAL] =
            year <= policy.coverageTerm ?
            data[index][AALA_RP]
            + (index > 0 ?
               data[index - 1][AALA_RP_TOTAL] : 0) :
            0;
            
            data[index][AALA_RP_PCTG_INVESTMENT_PORTION] = premiumAllocation[year - 1];
            data[index][AALA_RP_PCTG_ACQUISITION_COST] = 1
            - premiumAllocation[year - 1];
            
            data[index][AALA_RP_AMOUNT_ACQUISITION_COST] = data[index][AALA_RP]
            * data[index][AALA_RP_PCTG_ACQUISITION_COST];
            
            //AJ, AK, AL, AM
            data[index][AALA_BIAYA_POLIS_BULANAN] = monthlyPolicyFee[year - 1];
            
            data[index][AALA_PCTG_APH_CHARGES] = aphCharge[year - 1];
            
            data[index][AALA_TP_AD_HOC_PREMIUM] = (
                                                   durMonth % 12 == 1 ? policy.adhocTopUp[year - 1] : 0);
            
            data[index][AALA_TP_PCTG_INVESTMENT_PORTION] = topupAllocation[year
                                                                           - 1];
            data[index][AALA_TP_PCTG_ACQUISITION_COST] = 1
            - topupAllocation[year - 1];
            data[index][AALA_PCTG_SC_RP] = surrenderCharge[year - 1];
            data[index][AALA_AGE_POLICY_HOLDER] = payorAge;
            
            data[index][AALA_NLGI_APH] = policy.aphStart == 0 ? 0 :
            year < policy.aphStart ? 0 : 1;
        }
    }
    
    aala_calculateFunds(data, scenario);
    
    free(aphCharge);
    free(premiumAllocation);
    free(topupAllocation);
    free(surrenderCharge);
    free(monthlyPolicyFee);
    
    return data;
}












//get premium with freq
double aala_getFreqPremium(int minMax)
{
    double *atp = aala_getATPSubStandard();
    double freqPremium = 0;
    int i;
    for (i = 0; i < AALA_ATP_SIZE; i++)
    {
        if (minMax == AALA_MIN)
        {
            if (i != AALA_ATP_WOP_MAX && i != AALA_ATP_SW_MAX && i != AALA_ATP_PW_MAX && i != AALA_ATP_LIFE_MAX && i != AALA_ATP_SW_C3_MAX
                && i != AALA_ATP_WOP_C3_MAX)
            {
                freqPremium += (atp[i] / policy.freq);
            }
        }
        else
        {
            if (i != AALA_ATP_WOP_MIN && i != AALA_ATP_SW_MIN && i != AALA_ATP_PW_MIN && i != AALA_ATP_LIFE_MIN && i != AALA_ATP_SW_C3_MIN
                && i != AALA_ATP_WOP_C3_MIN)
            {
                freqPremium += (atp[i] / policy.freq);
            }
        }
    }
    
    free(atp);
    
    return freqPremium;
}

/*
 check validation of cash value
 */
int aala_isIllustrationValid()
{
    return aala_illustrationValid;
}

int aala_isAllowedWdrlPremium()
{
    return aala_allowedWdrlPremium;
}

double **aala_getData(int *row, int *col)
{
    aala_illustrationValid = TRUE;
    aala_allowedWdrlPremium = TRUE;
    aala_exchangeRate = aala_getExchangeRate(policy.prefix);
    
    int row_count, col_count;
    //    printf("low\n");
    double **dataLow = aala_calculateBasic(FUND_GROUP_LOW, &row_count, &col_count);
    double **dataMiddle = aala_calculateBasic(FUND_GROUP_MIDDLE, &row_count, &col_count);
    //    printf("middle\n");
    //    printf("high\n");
    double **dataHigh = aala_calculateBasic(FUND_GROUP_HIGH, &row_count, &col_count);
    
    double **data = make2DDoubleArray(policy.coverageTerm, AALA_SIZE);
    
    double *lien = aala_getLien(policy.prefix, NULL);
    double *adb = aala_getAdb(policy.prefix, NULL);
    double *indemnity = aala_getIndemnity(policy.prefix, NULL);
    //double *paBenefit = aala_getPABenefit(policy.prefix, NULL);
    
    double *maxWithdrawalValueLow = make1DDoubleArray(policy.coverageTerm);
    double *maxWithdrawalValueMiddle = make1DDoubleArray(policy.coverageTerm);
    double *maxWithdrawalValueHigh = make1DDoubleArray(policy.coverageTerm);
    int *validasiNLGI = make1DIntegerArray(policy.coverageTerm);
    
    if (row != NULL)
    {
        *row = policy.coverageTerm;
    }
    if (col != NULL)
    {
        *col = AALA_SIZE;
    }
    
    int i = 0;
    const int sustainMonthIndex = 10 * 12;
    double protectionCashValue = dataMiddle[sustainMonthIndex][AALA_RP_END_TOTAL];
    
    for (i = 0; i < policy.coverageTerm; i++)
    {
        const int year = i + 1;
        const int index = year * 12 - 1;
        const int durMonth = i * 12 + 1;
        
        data[i][AALA_YEAR] = i + 1; //0
        data[i][AALA_AGE] = dataLow[i * 12][AALA_AGE_INSURED] + 1; //1
        const int insuredAge = (int const) data[i][AALA_AGE];
        
        //		data[i][AALA_PERIODIC_PREMIUM] =
        //				(policy.aphStart == 0 || year < policy.aphStart) ?
        //						policy.annualPremium : 0; //3
        
        data[i][AALA_PERIODIC_PREMIUM] =
        dataHigh[durMonth - 1][AALA_RP] ? policy.annualPremium : 0; //3
        
        //		data[i][AALA_PERIODIC_TP_PREMIUM] =
        //				(policy.aphStart == 0 || year < policy.aphStart) ?
        //						dataHigh[durMonth - 1][AALA_TP_RP] * policy.freq
        //								+ dataHigh[durMonth - 1][AALA_TP_AD_HOC_PREMIUM] :
        //						0; //4
        
        //        data[i][AALA_PERIODIC_TP_PREMIUM] =
        //        dataHigh[durMonth - 1][AALA_RP] ?
        //        dataHigh[durMonth - 1][AALA_TP_RP] * policy.freq
        //        + dataHigh[durMonth - 1][AALA_TP_AD_HOC_PREMIUM] :
        //        0;
        //for TT571
        data[i][AALA_PERIODIC_TP_PREMIUM] =
        dataHigh[durMonth - 1][AALA_TP_RP] * policy.freq
        + dataHigh[durMonth - 1][AALA_TP_AD_HOC_PREMIUM]; //4
        
        data[i][AALA_WITHDRAWAL] = policy.regularWithdrawal[year - 1]
        + policy.topupWithdrawal[year - 1]; //5
        data[i][AALA_LOYALTY_BONUS_HIGH] = dataHigh[durMonth - 1][AALA_LOYALTY_BONUS] * policy.freq; //4
        
        // add by knight  excel  2016_08_10
        data[i][AALA_EXTRA_ALLOCATION_MIDDLE]  = dataMiddle[durMonth - 1][AALA_EXTRA_ALLOCATION] * policy.freq;
        
        data[i][AALA_PREMIUM_LOW] = dataLow[durMonth - 1][AALA_RP]
        + dataLow[durMonth - 1][AALA_TP_RP]; //6
        data[i][AALA_PREMIUM_MIDDLE] = dataMiddle[durMonth - 1][AALA_RP]
        + dataMiddle[durMonth - 1][AALA_TP_RP]; //7
        data[i][AALA_PREMIUM_HIGH] = dataHigh[durMonth - 1][AALA_RP]
        + dataHigh[durMonth - 1][AALA_TP_RP]; //8
        
        data[i][AALA_TOTAL_ACCOUNT_PROTECTION_LOW] =
        dataLow[index][AALA_RP_END_TOTAL]; //9
        data[i][AALA_TOTAL_ACCOUNT_PROTECTION_MIDDLE] =
        dataMiddle[index][AALA_RP_END_TOTAL]; //10
        data[i][AALA_TOTAL_ACCOUNT_PROTECTION_HIGH] =
        dataHigh[index][AALA_RP_END_TOTAL]; //11
        
        data[i][AALA_TOTAL_ACCOUNT_TOPUP_LOW] = dataLow[index][AALA_TP_END_TOTAL]; //12
        data[i][AALA_TOTAL_ACCOUNT_TOPUP_MIDDLE] =
        dataMiddle[index][AALA_TP_END_TOTAL]; //13
        data[i][AALA_TOTAL_ACCOUNT_TOPUP_HIGH] =
        dataHigh[index][AALA_TP_END_TOTAL]; //14
        
        data[i][AALA_TOTAL_ACCOUNT_LOW] = dataLow[index][AALA_RP_END_TOTAL] + dataLow[index][AALA_TP_END_TOTAL]; //15
        data[i][AALA_TOTAL_ACCOUNT_MIDDLE] = dataMiddle[index][AALA_RP_END_TOTAL] + dataMiddle[index][AALA_TP_END_TOTAL]; //16
        data[i][AALA_TOTAL_ACCOUNT_HIGH] = dataHigh[index][AALA_RP_END_TOTAL] + dataHigh[index][AALA_TP_END_TOTAL]; //17
        
        double ciPay = year <= ci.coverageTerm ? ci.sumAssured * lien[insuredAge - 1] : 0;
        //add by Bill Fu hardcode for urgent deadline.
        double abd_temp =0;
        double paAdbBenefit = 0;
        if(insuredAge<18){
            if(strcmp(policy.currency,"USD")==0){
                abd_temp = AALA_MAX_ADB_BENEFIT_USD_AGE_1;
                paAdbBenefit = AALA_COB_PA_USD_AGE_1;
            }else{
                abd_temp = AALA_MAX_ADB_BENEFIT_IDR_AGE_1;
                paAdbBenefit = AALA_COB_PA_IDR_AGE_1;
            }
        }else if(insuredAge<=70){
            if(strcmp(policy.currency,"USD")==0){
                abd_temp = AALA_MAX_ADB_BENEFIT_USD_AGE_18;
                paAdbBenefit = AALA_COB_PA_USD_AGE_18;
            }else{
                abd_temp = AALA_MAX_ADB_BENEFIT_IDR_AGE_18;
                paAdbBenefit = AALA_COB_PA_IDR_AGE_18;
            }
        }else{
            abd_temp = AALA_MAX_ADB_BENEFIT_IDR_AGE_70;
            paAdbBenefit = AALA_MAX_ADB_BENEFIT_IDR_AGE_70;
        }
        
        double paADB = fmin(pa.sumAssured, (paAdbBenefit - fmin(policy.sumAssured, abd_temp)));
        
        //        if(strcmp(policy.currency,"USD")==0){
        //            abd_temp =62500;
        //            if (insuredAge<18) {
        //                abd_temp =18750;
        //            }
        //        }else{
        //            abd_temp =500000000;
        //            if (insuredAge<18) {
        //                abd_temp =150000000;
        //            }
        //        }
        //        double paADB = fmin(pa.sumAssured, (paBenefit[insuredAge] - fmin(policy.sumAssured, abd_temp)));
        //double paIndemnity = fmin(pa.sumAssured, (paBenefit[insuredAge] - fmin(policy.sumAssured, indemnity[insuredAge - 1])));
       
        data[i][AALA_DEATH_BENEFIT_LOW] = dataLow[index][AALA_TOTAL_MANFAAT_RISIKO_MENINGGAL] ;//+ (dataLow[index][AALA_TOTAL_MANFAAT_RISIKO_MENINGGAL] > 0 ? ciPay : 0); //18
        data[i][AALA_DEATH_BENEFIT_MIDDLE] = dataMiddle[index][AALA_TOTAL_MANFAAT_RISIKO_MENINGGAL];// + (dataMiddle[index][AALA_TOTAL_MANFAAT_RISIKO_MENINGGAL] > 0 ? ciPay : 0); //19
        data[i][AALA_DEATH_BENEFIT_HIGH] = dataHigh[index][AALA_TOTAL_MANFAAT_RISIKO_MENINGGAL];// + (dataHigh[index][AALA_TOTAL_MANFAAT_RISIKO_MENINGGAL] > 0 ? ciPay : 0); //20
        
        data[i][AALA_ESTIMATED_OF_CASH_VALUE_LOW] = dataLow[index][AALA_ESTIMASI_NILAI_TEBUS]; //21
        data[i][AALA_ESTIMATED_OF_CASH_VALUE_MIDDLE] = dataMiddle[index][AALA_ESTIMASI_NILAI_TEBUS]; //22
        data[i][AALA_ESTIMATED_OF_CASH_VALUE_HIGH] = dataHigh[index][AALA_ESTIMASI_NILAI_TEBUS]; //23
        
        data[i][AALA_DEATH_BENEFIT_ADB_LOW] = dataLow[index][AALA_TOTAL_MANFAAT_RISIKO_MENINGGAL_ADB]; //24
        data[i][AALA_DEATH_BENEFIT_ADB_MIDDLE] = dataMiddle[index][AALA_TOTAL_MANFAAT_RISIKO_MENINGGAL_ADB] + ciPay + (paADB>=0?paADB:0); //25
        data[i][AALA_DEATH_BENEFIT_ADB_HIGH] = dataHigh[index][AALA_TOTAL_MANFAAT_RISIKO_MENINGGAL_ADB] + ciPay + (paADB>=0?paADB:0); //26
        
        data[i][AALA_DEATH_BENEFIT_INDEMNITY_LOW] = dataLow[index][AALA_TOTAL_MANFAAT_DOUBLE_INDEMNITY]/*+ paIndemnity*/; //27
        data[i][AALA_DEATH_BENEFIT_INDEMNITY_MIDDLE] = dataMiddle[index][AALA_TOTAL_MANFAAT_DOUBLE_INDEMNITY] + ciPay /*+ paIndemnity*/; //28
        data[i][AALA_DEATH_BENEFIT_INDEMNITY_HIGH] = dataHigh[index][AALA_TOTAL_MANFAAT_DOUBLE_INDEMNITY] + ciPay/* + paIndemnity*/; //29
        data[i][AALA_NO_LAPSE_GUARANTEE_INDICATOR] = dataMiddle[index][AALA_NLGI_VALIDASI];
        
        maxWithdrawalValueLow[i] = dataLow[index][AALA_MAXIMUM_WITHDRAWAL];
        maxWithdrawalValueMiddle[i] = dataMiddle[index][AALA_MAXIMUM_WITHDRAWAL];
        maxWithdrawalValueHigh[i] = dataHigh[index][AALA_MAXIMUM_WITHDRAWAL];
        validasiNLGI[i] = (int)dataMiddle[index][AALA_NLGI_VALIDASI];
    }
    
//    aala_illustrationValid = affp_illustrationChecking(data, policy, AALA_WITHDRAWAL, maxWithdrawalValueLow, maxWithdrawalValueMiddle, maxWithdrawalValueHigh, validasiNLGI,
//                                                       AALA_TOTAL_ACCOUNT_MIDDLE, protectionCashValue, AALA_AGE);
    
    
    
    //check withdrawl validation
    int hasAPH = policy.aphStart >= 11 ? 1 : 0;
    double min_total_AV_after_withdrawl = isSharedBasicIDR() ? 2500000 : 250;
    for (i = 0; i < policy.coverageTerm; i++)
    {
        double threeMonthsCharge;
        double totalWithdrawl;
        double total_AV_allow_withdrawl_Low;
        double maxWithdrawl;
        
        const int year = i + 1;
        const int index = year * 12 - 1;
        const int durMonth = i * 12 + 1;

        totalWithdrawl = data[i][AALA_WITHDRAWAL];
        threeMonthsCharge = dataLow[index][AALA_TOTAL_CHARGES] *3;
        total_AV_allow_withdrawl_Low =  year <= 5 ?
        dataLow[index][AALA_TOPUP_AV_AFTER_INVEST_TOTAL] :
        dataLow[index][AALA_TOPUP_AV_AFTER_INVEST_TOTAL] + dataLow[index][AALA_RP_END_TOTAL];
        
       maxWithdrawl = year == 0 ? 0 : (year <=5 ? total_AV_allow_withdrawl_Low : total_AV_allow_withdrawl_Low - maxDouble(threeMonthsCharge, min_total_AV_after_withdrawl));
        
        if (totalWithdrawl > maxWithdrawl)
        {
            aala_illustrationValid = WDWL_EXCEED_LIMIT;
            break;
        }
        
    }
    
    free(lien);
    free(adb);
    free(indemnity);
    //free(paBenefit);
    free(maxWithdrawalValueLow);
    free(maxWithdrawalValueMiddle);
    free(maxWithdrawalValueHigh);
    free(validasiNLGI);
    free2DDoubleArray(dataLow, row_count);
    free2DDoubleArray(dataMiddle, row_count);
    free2DDoubleArray(dataHigh, row_count);
    resetSharedVariables();
    
    return data;
}

double aala_getUnappliedPremium(struct basic policy, int age)
{
    return 0;
}


